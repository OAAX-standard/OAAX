#ifndef INTERFACE_H
#define INTERFACE_H

#include <stddef.h>
#include <stdio.h>
#include <unistd.h>

typedef enum tensor_data_type {
    DATA_TYPE_FLOAT = 1,
    DATA_TYPE_UINT8 = 2,
    DATA_TYPE_INT8 = 3,
    DATA_TYPE_UINT16 = 4,
    DATA_TYPE_INT16 = 5,
    DATA_TYPE_INT32 = 6,
    DATA_TYPE_INT64 = 7,
    DATA_TYPE_STRING = 8,
    DATA_TYPE_BOOL = 9,
    DATA_TYPE_DOUBLE = 11,
    DATA_TYPE_UINT32 = 12,
    DATA_TYPE_UINT64 = 13
} tensor_data_type;

typedef struct tensors_struct {
    size_t num_tensors;                 // Number of tensors
    char** names;                       // Names of the tensors
    tensor_data_type* data_types;       // Data types of the tensors
    size_t* ranks;                      // Ranks of the tensors
    size_t** shapes;                    // Shapes of the tensors
    void** data;                        // Data of the tensors
} tensors_struct;

/**
 * @brief This function is called only once to initialize the runtime environment.
 *
 * @return 0 if the initialization is successful, and non-zero otherwise.
 */
int runtime_initialization();

/**
 * @brief This function is called to initialize the runtime environment with arguments.
 * 
 * @note If this function is used to initialize the runtime, the runtime_initialization() function should not be called.
 * 
 * @note If an unknown key is passed, the runtime should ignore it.
 *
 * @param length The number of arguments.
 * @param keys The keys of the arguments.
 * @param values The values of the arguments.
 * @return 0 if the initialization is successful, and non-zero otherwise.
 */
int runtime_initialization_with_args( int length, const char **keys, const void **values );

/**
 * @brief This function is called to load the model from the file path.
 *
 * @param file_path The path to the model file.
 * @return 0 if the model is loaded successfully, and non-zero otherwise.
 */
int runtime_model_loading(const char *file_path);

/**
 * @brief This function is called to store the input tensors to be processed by the runtime when it's ready.
 * 
 * @note This function copies the reference of the input tensors, not the tensors themselves. 
 * The runtime will free the memory of the input tensors after its processed.
 * 
 * @warning If this function returns a non-zero value, the caller is expected to free the memory of the input tensors.
 *
 * @param tensors The input tensors for the inference processing. 
 * 
 * @return 0 if the input tensors are stored successfully, and non-zero otherwise.
 */
int send_input(tensors_struct *input_tensors);

/**
 * @brief This function is called to retrieve any available output tensors after the inference process is done.
 *
 * @note The caller is responsible for managing the memory of the output tensors.
 * 
 * @param output_tensors The output tensors of the inference process.
 * 
 * @return 0 if an output is available and returned, and non-zero otherwise.
 */
int receive_output(tensors_struct **output_tensors);

/**
 * @brief This function is called to destroy the runtime environment after the inference process is stopped.
 *
 * @return 0 if the finalization is successful, and non-zero otherwise.
 */
int runtime_destruction();

/**
 * @brief This function is called to get the error message in case of a runtime error.
 *
 * @return The error message in a human-readable format. This should be allocated by the shared library, and proper deallocation should be handled by the library.
 */
const char *runtime_error_message();

/**
 * @brief This function is called to get the version of the shared library.
 *
 * @return The version of the shared library. This should be allocated by the shared library, and proper deallocation should be handled by the library.
 */
const char *runtime_version();

/**
 * @brief This function is called to get the name of the shared library.
 *
 * @return The name of the shared library. This should be allocated by the shared library, and proper deallocation should be handled by the library.
 */
const char *runtime_name();

#endif // INTERFACE_H