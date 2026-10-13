#include <dlfcn.h>
#include <stdio.h>
#include <unistd.h>
#include "interface.h"

typedef int (*runtime_initialization_t)();
typedef int (*runtime_initialization_with_args_t)(int, const char *, const void **);
typedef int (*runtime_model_loading_t)(const char *);
typedef int (*send_input_t)(tensors_struct *);
typedef int (*receive_output_t)(tensors_struct **);
typedef int (*runtime_destruction_t)();
typedef const char *(*runtime_error_message_t)();
typedef const char *(*runtime_version_t)();
typedef const char *(*runtime_name_t)();

static void free_tensors_struct(tensors_struct *tensors);
void print_error_message(const char *error_message);

int main(int argc, char **argv) {
    if (argc != 3){
        print_error_message("./main <Runtime Library> <Model Path>\n");
        return 1;
    }
    printf("Usage: %s %s %s\n", argv[0], argv[1], argv[2]);
    char *runtime_path = argv[1];
    char *model_path = argv[2];

    void *handle;
    handle = dlopen(runtime_path, RTLD_NOW);
    if (!handle) {
        print_error_message(dlerror());
        return 1;
    }
    printf("Runtime library loaded successfully\n");

    runtime_initialization_t runtime_initialization = (runtime_initialization_t)dlsym(handle, "runtime_initialization");
    runtime_initialization_with_args_t runtime_initialization_with_args = (runtime_initialization_with_args_t)dlsym(handle, "runtime_initialization_with_args");
    runtime_model_loading_t runtime_model_loading = (runtime_model_loading_t)dlsym(handle, "runtime_model_loading");
    send_input_t send_input = (send_input_t)dlsym(handle, "send_input");
    receive_output_t receive_output = (receive_output_t)dlsym(handle, "receive_output");
    runtime_destruction_t runtime_destruction = (runtime_destruction_t)dlsym(handle, "runtime_destruction");
    runtime_error_message_t runtime_error_message = (runtime_error_message_t)dlsym(handle, "runtime_error_message");
    runtime_version_t runtime_version = (runtime_version_t)dlsym(handle, "runtime_version");
    runtime_name_t runtime_name = (runtime_name_t)dlsym(handle, "runtime_name");

    if (!runtime_initialization || !runtime_model_loading || !send_input || !receive_output || !runtime_destruction || !runtime_error_message || !runtime_version || !runtime_name) {
        print_error_message(dlerror());
        dlclose(handle);
        return 1;
    }
    printf("Runtime functions loaded successfully\n");

    // print runtime name and version
    const char *name = runtime_name();
    const char *version = runtime_version();
    printf("Runtime name: %s, version: %s\n", name, version);

    // initialize runtime environment
    if (runtime_initialization()) {
        print_error_message(runtime_error_message());
        dlclose(handle);
        return 1;
    }

    // load model
    const char *file_path = model_path;
    if (runtime_model_loading(file_path)) {
        print_error_message(runtime_error_message());
        dlclose(handle);
        return 1;
    }

    // send inputs
    for(int i=0; i<10; i++) {
        tensors_struct *tensors = malloc(sizeof(tensors_struct));
        memset(tensors, 0, sizeof(tensors_struct));
        tensors->num_tensors = 0; // empty input
        if (send_input(tensors)) {
            print_error_message(runtime_error_message());
            free_tensors_struct(tensors);
            continue;
        }
    }
    // receive outputs
    for(int i=0; i<10; i++) {
        tensors_struct *output_tensors = NULL;
        if (receive_output(&output_tensors)) {
            print_error_message(runtime_error_message());
            continue;
        }
        printf("Received output tensors\n");
        free_tensors_struct(output_tensors);
    }

    // finalize runtime environment
    if (runtime_destruction()) {
        print_error_message(runtime_error_message());
        dlclose(handle);
        return 1;
    }


    dlclose(handle);

    return 0;
}

static void free_tensors_struct(tensors_struct *tensors) {
  if (tensors->data_types != NULL) {
    free(tensors->data_types);
    tensors->data_types = NULL;
  }

  if (tensors->ranks != NULL) {
    free(tensors->ranks);
    tensors->ranks = NULL;
  }

  if (tensors->data != NULL) {
    for (size_t i = 0; i < tensors->num_tensors; i++) {
      if (tensors->data[i] != NULL)
        free(tensors->data[i]);
    }
    free(tensors->data);
    tensors->data = NULL;
  }

  if (tensors->shapes != NULL) {
    for (size_t i = 0; i < tensors->num_tensors; i++) {
      if (tensors->shapes[i] != NULL)
        free(tensors->shapes[i]);
    }
    free(tensors->shapes);
    tensors->shapes = NULL;
  }

  if (tensors->names != NULL) {
    for (size_t i = 0; i < tensors->num_tensors; i++) {
      if (tensors->names[i] != NULL)
        free(tensors->names[i]);
    }
    free(tensors->names);
    tensors->names = NULL;
  }

  free(tensors);
}

void print_error_message(const char *error_message) {
    fprintf(stderr, "Error: %s\n", error_message);
}
