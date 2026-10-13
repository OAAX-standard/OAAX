// Example OAAX 2.0 host: loads a runtime library, loads a model, sends a few
// inference requests and checks that every output matches a request id.
//
// Usage: ./host <runtime library> <model path>
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "interface.h"

#define NUM_REQUESTS 10
#define RETRIEVE_TIMEOUT_MS 1000

typedef struct {
  RuntimeStatus (*init)(Config);
  RuntimeStatus (*load_models)(int, const ModelConfig *);
  RuntimeStatus (*enqueue_input)(int, Tensors *);
  RuntimeStatus (*retrieve_output)(int *, Tensors **, int);
  RuntimeStatus (*cleanup)(void);
  const char *(*get_error)(void);
  const char *(*get_version)(void);
  const char *(*get_name)(void);
  const char *(*get_info)(void);
} Runtime;

static void free_tensors(Tensors *t) {
  if (!t)
    return;
  for (int i = 0; i < t->num_tensors; i++) {
    free(t->tensors[i].name);
    free(t->tensors[i].shape);
    free(t->tensors[i].data);
  }
  free(t->tensors);
  free(t);
}

// Builds one float input tensor of shape [1, 4] for request `id`.
static Tensors *make_input(int id) {
  Tensors *t = calloc(1, sizeof *t);
  if (!t)
    return NULL;
  t->id = id;
  t->tensors = calloc(1, sizeof *t->tensors);
  if (!t->tensors) {
    free(t);
    return NULL;
  }
  t->num_tensors = 1;
  TensorDescriptor *d = &t->tensors[0];
  d->name = strdup("input");
  d->shape = malloc(2 * sizeof *d->shape);
  d->data = calloc(4, sizeof(float));
  if (!d->name || !d->shape || !d->data) {
    free_tensors(t);
    return NULL;
  }
  d->data_type = DATA_TYPE_FLOAT;
  d->rank = 2;
  d->shape[0] = 1;
  d->shape[1] = 4;
  d->data_size = 4 * sizeof(float);
  return t;
}

static int load_runtime(const char *path, void **handle, Runtime *rt) {
  *handle = dlopen(path, RTLD_NOW | RTLD_LOCAL);
  if (!*handle) {
    fprintf(stderr, "Error: %s\n", dlerror());
    return 0;
  }
  // dlsym returns void*; the casts go through the member's own type.
#define RESOLVE(member, symbol)                                      \
  do {                                                               \
    *(void **)(&rt->member) = dlsym(*handle, symbol);                \
    if (!rt->member) {                                               \
      fprintf(stderr, "Error: missing symbol '%s'\n", symbol);       \
      dlclose(*handle);                                              \
      return 0;                                                      \
    }                                                                \
  } while (0)
  RESOLVE(init, "runtime_init");
  RESOLVE(load_models, "runtime_load_models");
  RESOLVE(enqueue_input, "runtime_enqueue_input");
  RESOLVE(retrieve_output, "runtime_retrieve_output");
  RESOLVE(cleanup, "runtime_cleanup");
  RESOLVE(get_error, "runtime_get_error");
  RESOLVE(get_version, "runtime_get_version");
  RESOLVE(get_name, "runtime_get_name");
  RESOLVE(get_info, "runtime_get_info");
#undef RESOLVE
  return 1;
}

static void report(const Runtime *rt, const char *what, RuntimeStatus status) {
  const char *msg = rt->get_error();
  fprintf(stderr, "Error: %s failed (status %d): %s\n", what, status, msg ? msg : "n/a");
}

int main(int argc, char **argv) {
  if (argc != 3) {
    fprintf(stderr, "Usage: %s <runtime library> <model path>\n", argv[0]);
    return 1;
  }

  void *handle = NULL;
  Runtime rt;
  if (!load_runtime(argv[1], &handle, &rt))
    return 1;
  printf("Runtime: %s %s\n", rt.get_name(), rt.get_version());

  int exit_code = 1;
  int seen[NUM_REQUESTS] = {0};
  RuntimeStatus status;

  // 1. Initialize. Config values are always strings; this runtime takes no options.
  Config config = {0, NULL, NULL};
  if ((status = rt.init(config)) != RUNTIME_STATUS_SUCCESS) {
    report(&rt, "runtime_init", status);
    goto unload;
  }

  // 2. Load the model(s). The model's id is its index in this array.
  ModelConfig model = {.file_path = argv[2], .model_data = NULL, .model_size = 0, .config = {0, NULL, NULL}};
  if ((status = rt.load_models(1, &model)) != RUNTIME_STATUS_SUCCESS) {
    report(&rt, "runtime_load_models", status);
    goto cleanup;
  }
  printf("Info: %s\n", rt.get_info());

  // 3. Enqueue requests. On success the runtime owns the input; on failure we free it.
  for (int id = 0; id < NUM_REQUESTS; id++) {
    Tensors *input = make_input(id);
    if (!input) {
      fprintf(stderr, "Error: out of memory\n");
      goto cleanup;
    }
    if ((status = rt.enqueue_input(0, input)) != RUNTIME_STATUS_SUCCESS) {
      report(&rt, "runtime_enqueue_input", status);
      free_tensors(input);
      goto cleanup;
    }
  }

  // 4. Retrieve outputs; the Tensors.id ties each one back to its request.
  for (int received = 0; received < NUM_REQUESTS; received++) {
    int model_id = -1;
    Tensors *output = NULL;
    status = rt.retrieve_output(&model_id, &output, RETRIEVE_TIMEOUT_MS);
    if (status != RUNTIME_STATUS_SUCCESS) {
      report(&rt, "runtime_retrieve_output", status);
      goto cleanup;
    }
    if (output->id < 0 || output->id >= NUM_REQUESTS || seen[output->id]) {
      fprintf(stderr, "Error: unexpected or duplicate output id %d\n", output->id);
      free_tensors(output);
      goto cleanup;
    }
    seen[output->id] = 1;
    printf("Output for request %d (model %d): %d tensor(s), first is '%s' with rank %d\n", output->id, model_id,
           output->num_tensors, output->tensors[0].name, output->tensors[0].rank);
    free_tensors(output); // the caller owns outputs
  }

  printf("OK: %d/%d requests answered\n", NUM_REQUESTS, NUM_REQUESTS);
  exit_code = 0;

cleanup:
  if ((status = rt.cleanup()) != RUNTIME_STATUS_SUCCESS) {
    report(&rt, "runtime_cleanup", status);
    exit_code = 1;
  }
unload:
  dlclose(handle);
  return exit_code;
}
