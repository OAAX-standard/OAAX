#include <dlfcn.h>
#include <stdio.h>
#include <unistd.h>
#include "interface.h"

typedef int (*runtime_initialization_t)();
typedef int (*runtime_model_loading_t)(const char *);
typedef int (*runtime_inference_execution_t)(tensors_struct *, tensors_struct *);
typedef int (*runtime_inference_cleanup_t)();
typedef int (*runtime_destruction_t)();
typedef const char *(*runtime_error_message_t)();
typedef const char *(*runtime_version_t)();
typedef const char *(*runtime_name_t)();

void print_error_message(const char *error_message) {
    fprintf(stderr, "Error: %s\n", error_message);
}

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
    runtime_model_loading_t runtime_model_loading = (runtime_model_loading_t)dlsym(handle, "runtime_model_loading");
    runtime_inference_execution_t runtime_inference_execution = (runtime_inference_execution_t)dlsym(handle, "runtime_inference_execution");
    runtime_inference_cleanup_t runtime_inference_cleanup = (runtime_inference_cleanup_t)dlsym(handle, "runtime_inference_cleanup");
    runtime_destruction_t runtime_destruction = (runtime_destruction_t)dlsym(handle, "runtime_destruction");
    runtime_error_message_t runtime_error_message = (runtime_error_message_t)dlsym(handle, "runtime_error_message");
    runtime_version_t runtime_version = (runtime_version_t)dlsym(handle, "runtime_version");
    runtime_name_t runtime_name = (runtime_name_t)dlsym(handle, "runtime_name");

    if (!runtime_initialization || !runtime_model_loading || !runtime_inference_execution || !runtime_inference_cleanup || !runtime_destruction || !runtime_error_message || !runtime_version || !runtime_name) {
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

    // execute model
    tensors_struct input_tensors, output_tensors;
    while(1){
        printf("\n");

        input_tensors.num_tensors = 1;
        if (runtime_inference_execution(&input_tensors, &output_tensors)) {
            print_error_message(runtime_error_message());
            break;
        }

        sleep(1); // Simulate the inference process

        printf("Number of output tensors: %ld\n", output_tensors.num_tensors);

        // free output message
        if (runtime_inference_cleanup()) {
            print_error_message(runtime_error_message());
            break;
        }
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