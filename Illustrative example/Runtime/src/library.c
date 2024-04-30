#include "interface.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include <unistd.h>


 const char *_runtime_name ="ExampleRuntime";
static const char *_runtime_version = "0.1.0";
static char *_error_message = NULL;
static tensors_struct _output_tensors;

// Helper function to raise an error randomly to simulate an occasional runtime error
static int raise_error();
// Helper function to build the error message object from a string
static void build_error_message(const char *error_message);

/****************************************************
 * Core runtime functions
 ****************************************************/
const char *runtime_name(){
    return _runtime_name;
}

const char *runtime_version(){
    return _runtime_version;
}

const char *runtime_error_message(){
    return _error_message;
}


int runtime_initialization(){
    srand(time(NULL)); // Seed the random number generator

    printf("Initializing the runtime environment\n");
    if (raise_error()){
        build_error_message("Failed to initialize the runtime environment");
        return 1;
    }
    return 0;
}

int runtime_model_loading(const char *file_path){
    if (access(file_path, F_OK) == 0) {
        printf("Loading the model from the file path: %s\n", file_path);
    } else {
        build_error_message("Model file does not exist");
        return 2;
    }
    if (raise_error()){
        build_error_message("Failed to load the model");
        return 1;
    }
    return 0;
}

int runtime_inference_execution(tensors_struct *input_tensors, tensors_struct *output_tensors){
    printf("Running the model on the input tenors\n");

    // print number of tensors
    printf("Number of input tensors: %ld\n", input_tensors->num_tensors);
    // check if the input tensors are valid
    if (input_tensors->num_tensors == 0){
        build_error_message("No input tensors provided");
        return 1;
    }

    // Create one dummy output tensor
    int n_tensors = 1;
    _output_tensors.num_tensors = n_tensors;
    _output_tensors.ranks = (size_t *)malloc(n_tensors * sizeof(size_t));
    _output_tensors.shapes = (size_t **)malloc(n_tensors * sizeof(size_t *));
    _output_tensors.data_types = (tensor_data_type *)malloc(n_tensors * sizeof(tensor_data_type));
    _output_tensors.data = (void **)malloc(n_tensors * sizeof(void *));
    _output_tensors.names = NULL;

    _output_tensors.ranks[0] = 2;
    _output_tensors.shapes[0] = (size_t *) malloc(_output_tensors.ranks[0] * sizeof(size_t));
    _output_tensors.shapes[0][0] = 1;
    _output_tensors.shapes[0][1] = 10;
    _output_tensors.data_types[0] = DATA_TYPE_FLOAT;
    _output_tensors.data[0] = (float *)malloc(10 * sizeof(float));

    if(raise_error()){
        build_error_message("Failed to run inference with the provided input tensors");
        return 2;
    }

    // Assign the output tensors to the output_tensors pointer
    *output_tensors = _output_tensors;

    return 0;
}

int runtime_inference_cleanup(){
    printf("Cleaning up the output tensors and other resources\n");
    if (_output_tensors.num_tensors > 0){
        for (int i = 0; i < _output_tensors.num_tensors; i++){
            free(_output_tensors.shapes[i]);
            free(_output_tensors.data[i]);
        }
        free(_output_tensors.ranks);
        free(_output_tensors.data_types);
        free(_output_tensors.shapes);
        free(_output_tensors.data);
    } else {
        build_error_message("No output tensor was produced by the inference execution");
        return 1;
    }
    return 0;
}

int runtime_destruction(){
    printf("Destroying the runtime environment\n");
    // Clean up the output packed message
    runtime_inference_cleanup();

    // Clean up the error message
    if (_error_message != NULL){
        free(_error_message);
        _error_message = NULL;
    }
    return 0;
}

/****************************************************
 * Helper functions
 ****************************************************/

static int raise_error(){
    int r = rand();
    if (r % 10 == 0) { // Raise an error 10% of the time
        printf("* Runtime randomly raising an error.\n");
        return 1;
    }else
        return 0;
}
static void build_error_message(const char *error_message){
    if(_error_message != NULL){
        free(_error_message);
        _error_message = NULL;
    }
    _error_message = (char *)malloc(strlen(error_message));
    strcpy(_error_message, error_message);
}