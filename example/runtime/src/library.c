#include "interface.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

const char *_runtime_name = "ExampleRuntime";
static const char *_runtime_version = "0.2.0";
static char *_error_message = NULL;

// Helper function to raise an error randomly to simulate an occasional runtime
// error
static int raise_error();
// Helper function to build the error message object from a string
static void build_error_message(const char *error_message);
// Helper function to free the memory of the tensors_struct object
static void free_tensors_struct(tensors_struct *tensors);

/****************************************************
 * Core runtime functions
 ****************************************************/
const char *runtime_name() { return _runtime_name; }

const char *runtime_version() { return _runtime_version; }

const char *runtime_error_message() { return _error_message; }

int runtime_initialization_with_args(int length, const char **keys,
                                     const void **values) {
  printf("Initializing the runtime environment with arguments\n");
  if (raise_error()) {
    build_error_message("Failed to initialize the runtime environment");
    return 1;
  }

  for (int i = 0; i < length; i++) {
    if (raise_error()) {
      printf("Unknown key '%s'\n", keys[i]);
      continue;
    }
    printf("Using Key: %s\n", keys[i]);
  }

  return runtime_initialization();
}

int runtime_initialization() {
  srand(time(NULL)); // Seed the random number generator

  printf("Initializing the runtime environment\n");
  if (raise_error()) {
    build_error_message("Failed to initialize the runtime environment");
    return 1;
  }
  return 0;
}

int runtime_model_loading(const char *file_path) {
  if (access(file_path, F_OK) == 0) {
    printf("Loading the model from the file path: %s\n", file_path);
  } else {
    build_error_message("Model file does not exist");
    return 2;
  }
  if (raise_error()) {
    build_error_message("Failed to load the model");
    return 1;
  }
  return 0;
}

int send_input(tensors_struct *input_tensors) {
  printf("Storing the input tensors for inference\n");

  // print number of tensors
  printf("Received %ld tensors\n", input_tensors->num_tensors);

  if (raise_error()) {
    build_error_message("Failed to store the input tensors");
    return 2;
  }

  printf("Input tensors stored successfully\n");

  free_tensors_struct(input_tensors); // Free the input tensors

  return 0;
}

int receive_output(tensors_struct **output_tensors) {
  if (raise_error()) {
    build_error_message("There's no output tensors available");
    return 2;
  }
  printf("Retrieving the output tensors\n");

  // Create one dummy output tensor
  int n_tensors = 1;
  tensors_struct *_output_tensors =
      (tensors_struct *)malloc(sizeof(tensors_struct));
  _output_tensors->num_tensors = n_tensors;
  _output_tensors->ranks = (size_t *)malloc(n_tensors * sizeof(size_t));
  _output_tensors->shapes = (size_t **)malloc(n_tensors * sizeof(size_t *));
  _output_tensors->data_types =
      (tensor_data_type *)malloc(n_tensors * sizeof(tensor_data_type));
  _output_tensors->data = (void **)malloc(n_tensors * sizeof(void *));
  _output_tensors->names = NULL;

  _output_tensors->ranks[0] = 2;
  _output_tensors->shapes[0] =
      (size_t *)malloc(_output_tensors->ranks[0] * sizeof(size_t));
  _output_tensors->shapes[0][0] = 1;
  _output_tensors->shapes[0][1] = 10;
  _output_tensors->data_types[0] = DATA_TYPE_FLOAT;
  _output_tensors->data[0] = (float *)malloc(10 * sizeof(float));

  // Assign the output tensors to the output_tensors pointer
  *output_tensors = _output_tensors;

  printf("Output tensors returned successfully\n");

  return 0;
}

int runtime_destruction() {
  printf("Destroying the runtime environment\n");

  // Clean up the error message
  if (_error_message != NULL) {
    free(_error_message);
    _error_message = NULL;
  }
  return 0;
}

/****************************************************
 * Helper functions
 ****************************************************/

static int raise_error() {
  int r = rand();
  if (r % 10 == 0) { // Raise an error 10% of the time
    printf("* Runtime randomly raising an error.\n");
    return 1;
  } else
    return 0;
}
static void build_error_message(const char *error_message) {
  if (_error_message != NULL) {
    free(_error_message);
    _error_message = NULL;
  }
  _error_message = (char *)malloc(strlen(error_message));
  strcpy(_error_message, error_message);
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