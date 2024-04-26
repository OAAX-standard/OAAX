#ifndef INTERFACE_H
#define INTERFACE_H

#include <stddef.h>

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
    const char** names;                 // Names of the tensors
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
 * @brief This function is called to load the model from the file path.
 *
 * @param file_path The path to the model file.
 * @return 0 if the model is loaded successfully, and non-zero otherwise.
 */
int runtime_model_loading(const char *file_path);

/**
 * @brief This function is called to execute the model on the input tensors.
 *
 * @param input_tensors The input tensors to feed to the model. Note that the input tensors are completely managed by the caller (both allocation and freeing).
 * @param output_tensors The output tensors computed during inference. Note that the output tensors are completely managed by this function (both allocation and freeing).
 * @return 0 if the execution is successful, and non-zero otherwise.
 */
int runtime_inference_execution(tensors_struct *input_tensors, tensors_struct *output_tensors);

/**
 * @brief This function is called after each inference run to clean up the output tensors and any other resources if needed.
 *
 * @return 0 if the cleanup is successful, and non-zero otherwise.
 */
int runtime_inference_cleanup();

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