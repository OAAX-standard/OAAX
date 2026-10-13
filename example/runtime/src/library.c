// Minimal OAAX 2.0 runtime. It does no real inference: every enqueued input
// produces one dummy output (zeros, shape [1, 10]) carrying the same Tensors.id.
// It exists to show the call contract: lifecycle, ownership, async queue,
// timeouts, thread safety and error reporting.
#include "interface.h"

#include <errno.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define OUTPUT_CLASSES 10

typedef struct Request {
  int model_id;
  Tensors *input;
  struct Request *next;
} Request;

static pthread_mutex_t g_lock = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t g_ready = PTHREAD_COND_INITIALIZER;
static int g_initialized = 0;
static int g_num_models = 0;
static int g_pending = 0;
static Request *g_head = NULL, *g_tail = NULL;
static char g_error[256] = "";
static char g_info[256] = "";

static RuntimeStatus fail(RuntimeStatus status, const char *message) {
  snprintf(g_error, sizeof g_error, "%s", message);
  return status;
}

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

// Builds the dummy output for a request: a single zeroed [1, OUTPUT_CLASSES] tensor.
static Tensors *make_output(int id) {
  Tensors *out = calloc(1, sizeof *out);
  if (!out)
    return NULL;
  out->id = id; // correlation contract: echo the input's id
  out->tensors = calloc(1, sizeof *out->tensors);
  if (!out->tensors) {
    free(out);
    return NULL;
  }
  out->num_tensors = 1;
  TensorDescriptor *d = &out->tensors[0];
  d->name = strdup("output");
  d->shape = malloc(2 * sizeof *d->shape);
  d->data_size = OUTPUT_CLASSES * sizeof(float);
  d->data = calloc(OUTPUT_CLASSES, sizeof(float));
  if (!d->name || !d->shape || !d->data) {
    free_tensors(out);
    return NULL;
  }
  d->data_type = DATA_TYPE_FLOAT;
  d->rank = 2;
  d->shape[0] = 1;
  d->shape[1] = OUTPUT_CLASSES;
  return out;
}

// Caller must hold g_lock.
static void drop_pending(void) {
  while (g_head) {
    Request *r = g_head;
    g_head = r->next;
    free_tensors(r->input);
    free(r);
  }
  g_tail = NULL;
  g_pending = 0;
}

static int input_is_valid(const Tensors *t) {
  if (!t || t->num_tensors < 0 || (t->num_tensors > 0 && !t->tensors))
    return 0;
  for (int i = 0; i < t->num_tensors; i++)
    if (!t->tensors[i].name || !t->tensors[i].data || t->tensors[i].rank < 0 ||
        (t->tensors[i].rank > 0 && !t->tensors[i].shape))
      return 0;
  return 1;
}

/****************************************************
 * Lifecycle
 ****************************************************/

RuntimeStatus runtime_init(Config config) {
  if (config.length < 0 || (config.length > 0 && (!config.keys || !config.values)))
    return fail(RUNTIME_STATUS_INVALID_ARGUMENT, "Invalid configuration");

  pthread_mutex_lock(&g_lock);
  if (g_initialized) {
    pthread_mutex_unlock(&g_lock);
    return fail(RUNTIME_STATUS_ALREADY_INITIALIZED, "Runtime is already initialized");
  }
  g_initialized = 1;
  g_num_models = 0;
  g_error[0] = '\0';
  pthread_mutex_unlock(&g_lock);

  for (int i = 0; i < config.length; i++) // this runtime has no options: unknown keys are ignored
    printf("[runtime] ignoring config key '%s'\n", config.keys[i]);
  return RUNTIME_STATUS_SUCCESS;
}

RuntimeStatus runtime_load_models(int num_models, const ModelConfig *model_configs) {
  if (num_models <= 0 || !model_configs)
    return fail(RUNTIME_STATUS_INVALID_ARGUMENT, "No models to load");

  pthread_mutex_lock(&g_lock);
  RuntimeStatus status = RUNTIME_STATUS_SUCCESS;
  if (!g_initialized) {
    status = fail(RUNTIME_STATUS_NOT_INITIALIZED, "Call runtime_init() first");
  } else if (g_num_models > 0) {
    status = fail(RUNTIME_STATUS_ERROR, "Models are already loaded");
  } else {
    for (int i = 0; i < num_models && status == RUNTIME_STATUS_SUCCESS; i++) {
      const ModelConfig *m = &model_configs[i];
      if (m->file_path) {
        if (access(m->file_path, R_OK) != 0)
          status = fail(RUNTIME_STATUS_FILE_NOT_FOUND, "Model file does not exist");
        else
          printf("[runtime] model %d: loading '%s'\n", i, m->file_path);
      } else if (m->model_data && m->model_size > 0) {
        printf("[runtime] model %d: loading %zu bytes from memory\n", i, m->model_size);
      } else {
        status = fail(RUNTIME_STATUS_INVALID_ARGUMENT, "Model needs a file_path or model_data");
      }
    }
    if (status == RUNTIME_STATUS_SUCCESS)
      g_num_models = num_models; // a real runtime would copy whatever it needs from model_configs here
  }
  pthread_mutex_unlock(&g_lock);
  return status;
}

RuntimeStatus runtime_cleanup(void) {
  pthread_mutex_lock(&g_lock);
  drop_pending();
  g_initialized = 0; // idempotent: calling this again is a no-op
  g_num_models = 0;
  g_error[0] = '\0';
  pthread_mutex_unlock(&g_lock);
  return RUNTIME_STATUS_SUCCESS;
}

/****************************************************
 * Inference
 ****************************************************/

RuntimeStatus runtime_enqueue_input(int model_id, Tensors *input_tensors) {
  // On any failure the caller keeps ownership of input_tensors.
  if (!input_is_valid(input_tensors))
    return fail(RUNTIME_STATUS_INVALID_TENSOR, "Invalid input tensors");

  Request *r = malloc(sizeof *r);
  if (!r)
    return fail(RUNTIME_STATUS_OUT_OF_MEMORY, "Out of memory");

  pthread_mutex_lock(&g_lock);
  RuntimeStatus status = RUNTIME_STATUS_SUCCESS;
  if (!g_initialized)
    status = fail(RUNTIME_STATUS_NOT_INITIALIZED, "Call runtime_init() first");
  else if (g_num_models == 0)
    status = fail(RUNTIME_STATUS_MODEL_NOT_LOADED, "No model loaded");
  else if (model_id < 0 || model_id >= g_num_models)
    status = fail(RUNTIME_STATUS_INVALID_MODEL_ID, "Invalid model id");

  if (status != RUNTIME_STATUS_SUCCESS) {
    pthread_mutex_unlock(&g_lock);
    free(r);
    return status;
  }

  r->model_id = model_id;
  r->input = input_tensors; // ownership moves to the runtime
  r->next = NULL;
  if (g_tail)
    g_tail->next = r;
  else
    g_head = r;
  g_tail = r;
  g_pending++;
  pthread_cond_signal(&g_ready);
  pthread_mutex_unlock(&g_lock);
  return RUNTIME_STATUS_SUCCESS;
}

RuntimeStatus runtime_retrieve_output(int *model_id, Tensors **output_tensors, int timeout_ms) {
  if (!model_id || !output_tensors)
    return fail(RUNTIME_STATUS_INVALID_ARGUMENT, "model_id and output_tensors must not be NULL");

  struct timespec deadline;
  if (timeout_ms > 0) {
    clock_gettime(CLOCK_REALTIME, &deadline);
    deadline.tv_sec += timeout_ms / 1000;
    deadline.tv_nsec += (long)(timeout_ms % 1000) * 1000000L;
    if (deadline.tv_nsec >= 1000000000L) {
      deadline.tv_sec++;
      deadline.tv_nsec -= 1000000000L;
    }
  }

  pthread_mutex_lock(&g_lock);
  if (!g_initialized) {
    pthread_mutex_unlock(&g_lock);
    return fail(RUNTIME_STATUS_NOT_INITIALIZED, "Call runtime_init() first");
  }
  // timeout_ms == 0: don't wait; < 0: wait forever; > 0: wait until the deadline.
  while (!g_head && timeout_ms != 0) {
    int rc = (timeout_ms < 0) ? pthread_cond_wait(&g_ready, &g_lock)
                              : pthread_cond_timedwait(&g_ready, &g_lock, &deadline);
    if (rc == ETIMEDOUT)
      break;
  }
  if (!g_head) {
    pthread_mutex_unlock(&g_lock);
    return fail(RUNTIME_STATUS_NO_OUTPUT_AVAILABLE, "No output available");
  }

  Request *r = g_head;
  g_head = r->next;
  if (!g_head)
    g_tail = NULL;
  g_pending--;
  pthread_mutex_unlock(&g_lock);

  // "Inference" happens here: the input is consumed and freed, then the output is built.
  Tensors *out = make_output(r->input->id);
  int id = r->model_id;
  free_tensors(r->input);
  free(r);
  if (!out)
    return fail(RUNTIME_STATUS_OUT_OF_MEMORY, "Out of memory");

  *model_id = id;
  *output_tensors = out; // ownership moves to the caller
  return RUNTIME_STATUS_SUCCESS;
}

/****************************************************
 * Getters
 ****************************************************/

const char *runtime_get_error(void) { return g_error[0] ? g_error : NULL; }

const char *runtime_get_version(void) { return "2.0.0"; }

const char *runtime_get_name(void) { return "OAAX Example Runtime"; }

const char *runtime_get_info(void) {
  pthread_mutex_lock(&g_lock);
  if (!g_initialized) {
    pthread_mutex_unlock(&g_lock);
    return NULL;
  }
  snprintf(g_info, sizeof g_info,
           "{\"active_device\":\"CPU\",\"loaded_models\":%d,\"requests_in_flight\":%d}",
           g_num_models, g_pending);
  pthread_mutex_unlock(&g_lock);
  return g_info;
}
