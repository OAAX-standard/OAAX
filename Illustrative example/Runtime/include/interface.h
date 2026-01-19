#ifndef OAAX_RUNTIME_INTERFACE_H
#define OAAX_RUNTIME_INTERFACE_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Status of the runtime.
 *
 * @note The data types of the tensors are borrowed from the ONNX data type
 * enum.
 * @see
 * https://onnx.ai/ir-py/api/generated/onnx_ir.DataType.html#onnx_ir.DataType.UNDEFINED
 */
typedef enum RuntimeStatus {
  RUNTIME_STATUS_SUCCESS = 0,
  RUNTIME_STATUS_ERROR = 1,
  RUNTIME_STATUS_NOT_INITIALIZED = 2,
  RUNTIME_STATUS_ALREADY_INITIALIZED = 3,
  RUNTIME_STATUS_MODEL_NOT_LOADED = 4,
  RUNTIME_STATUS_INVALID_ARGUMENT = 5,
  RUNTIME_STATUS_INVALID_MODEL = 6,
  RUNTIME_STATUS_FILE_NOT_FOUND = 7,
  RUNTIME_STATUS_OUT_OF_MEMORY = 8,
  RUNTIME_STATUS_INVALID_TENSOR = 9,
  RUNTIME_STATUS_TENSOR_SHAPE_MISMATCH = 10,
  RUNTIME_STATUS_TENSOR_TYPE_MISMATCH = 11,
  RUNTIME_STATUS_NO_OUTPUT_AVAILABLE = 12,
  RUNTIME_STATUS_INFERENCE_ERROR = 13,
  RUNTIME_STATUS_NOT_IMPLEMENTED = 14,
  RUNTIME_STATUS_TIMEOUT = 15,
  RUNTIME_STATUS_DEVICE_ERROR = 16,
  RUNTIME_STATUS_UNKNOWN_ERROR = 17,
  RUNTIME_STATUS_INVALID_MODEL_ID = 18
} RuntimeStatus;

/**
 * @brief Data type of the tensor elements.
 */
typedef enum TensorElementType {
  DATA_TYPE_UNDEFINED = 0,
  DATA_TYPE_FLOAT = 1,
  DATA_TYPE_UINT8 = 2,
  DATA_TYPE_INT8 = 3,
  DATA_TYPE_UINT16 = 4,
  DATA_TYPE_INT16 = 5,
  DATA_TYPE_INT32 = 6,
  DATA_TYPE_INT64 = 7,
  DATA_TYPE_STRING = 8,
  DATA_TYPE_BOOL = 9,
  DATA_TYPE_FLOAT16 = 10,
  DATA_TYPE_DOUBLE = 11,
  DATA_TYPE_UINT32 = 12,
  DATA_TYPE_UINT64 = 13,
  DATA_TYPE_COMPLEX64 = 14,
  DATA_TYPE_COMPLEX128 = 15,
  DATA_TYPE_BFLOAT16 = 16,
  DATA_TYPE_FLOAT8E4M3FN = 17,
  DATA_TYPE_FLOAT8E4M3FNUZ = 18,
  DATA_TYPE_FLOAT8E5M2 = 19,
  DATA_TYPE_FLOAT8E5M2FNUZ = 20,
  DATA_TYPE_UINT4 = 21,
  DATA_TYPE_INT4 = 22,
  DATA_TYPE_FLOAT4E2M1 = 23,
  DATA_TYPE_FLOAT8E8M0 = 24,
  DATA_TYPE_UINT2 = 25,
  DATA_TYPE_INT2 = 26
} TensorElementType;

/**
 * @brief Structure to hold the tensors.
 */
typedef struct Tensors {
  int id;                        // ID of the tensor
  int num_tensors;               // Number of tensors
  char **names;                  // Names of the tensors
  TensorElementType *data_types; // Data types of the tensors
  int *ranks;                    // Ranks of the tensors
  int **shapes;                  // Shapes of the tensors
  void **data;                   // Data of the tensors
} Tensors;

/**
 * @brief Configuration structure for key-value arguments.
 *
 * Used for both runtime initialization and model-specific configuration.
 */
typedef struct Config {
  int length;          // Number of configuration arguments
  const char **keys;   // Keys of the configuration arguments
  const void **values; // Values of the configuration arguments
} Config;

/**
 * @brief Configuration structure for a model.
 *
 * Contains the model file path and its configuration arguments.
 */
typedef struct ModelConfig {
  const char *file_path; // Path to the model file
  Config config;         // Configuration of the model
} ModelConfig;

/**
 * @brief Initialize the runtime environment with configuration arguments.
 *
 * @param config Configuration structure containing initialization arguments.
 * @return RuntimeStatus indicating success or failure.
 *
 * @note Caller responsibility:
 * - Must provide a valid Config structure with properly allocated keys and values arrays.
 * - Must ensure config.keys and config.values arrays are valid and have length matching config.length.
 * - Must not call this function more than once without calling runtime_cleanup() first.
 * - Must handle initialization failures appropriately.
 *
 * @note Runtime responsibility:
 * - Must initialize all runtime resources and prepare the environment for model loading.
 * - Must ignore unknown configuration keys without error.
 * - Must return RUNTIME_STATUS_SUCCESS on successful initialization.
 * - Must return appropriate error status codes on failure.
 * - Must be robust to multiple initialization calls.
 */
RuntimeStatus runtime_init(Config config);

/**
 * @brief Load multiple models with their configurations.
 *
 * @param num_models The number of models to load.
 * @param model_configs Array of model configurations. Each config contains the
 * file path and model-specific configuration.
 * @return RuntimeStatus indicating success or failure.
 *
 * @note Models are identified by their index in the array (0, 1, 2, ...) when
 * calling runtime_enqueue_input and runtime_retrieve_output.
 *
 * @note Caller responsibility:
 * - Must ensure runtime_init() has been called successfully before calling this function.
 * - Must provide a valid array of ModelConfig structures with num_models elements.
 * - Must ensure each ModelConfig.file_path points to a valid, accessible model file.
 * - Must ensure each ModelConfig.config has valid keys and values arrays if length > 0.
 * - Must ensure that config list is accessible and valid while the function is executing.
 * - If a model fails to load, the caller can retry loading another config of models without having to re-call the runtime_init function.
 *
 * @note Runtime responsibility:
 * - Must load and initialize all models specified in the array.
 * - Must return RUNTIME_STATUS_SUCCESS if all models load successfully.
 * - Must return appropriate error status if any model fails to load.
 * - Must maintain loaded models until runtime_cleanup() is called.
 * - Must validate model files and configurations before loading.
 * - Must free all resources allocated during the call if any model fails to load.
 * - Mustn't assume that the model_configs array is accessible and valid after the call.
 */
RuntimeStatus runtime_load_models(int num_models,
                                 const ModelConfig *model_configs);

/**
 * @brief Enqueue input tensors for inference on a specific model.
 *
 * @param model_id The identifier of the model to use for inference (index in
 * the models array from runtime_load_models).
 * @param input_tensors The input tensors for the inference processing.
 * @return RuntimeStatus indicating success or failure.
 *
 * @note This function enqueues the input tensors for asynchronous processing.
 * The inference may execute immediately or be queued for later execution.
 *
 * @note Caller responsibility:
 * - Must ensure the model_id corresponds to a successfully loaded model.
 * - Must provide a valid Tensors structure with all fields properly initialized.
 * - Must ensure all arrays (names, data_types, ranks, shapes, data) are valid and match num_tensors.
 * - Must ensure tensor data matches the specified data_types and shapes.
 * - Must transfer ownership of the input_tensors structure to the runtime.
 * - If this function fails for any reason, the caller is responsible for freeing the input_tensors structure.
 *
 * @note Runtime responsibility:
 * - Must validate model_id is within valid range and corresponds to a loaded model.
 * - Must validate input_tensors structure and all its fields.
 * - Must validate tensor shapes and types match the model's expected inputs.
 * - Must copy references to input_tensors (not deep copy the data).
 * - Must take ownership of input_tensors memory upon successful enqueue (RUNTIME_STATUS_SUCCESS).
 * - Must free all memory associated with input_tensors after inference completes.
 * - Must not free input_tensors memory if function fails for any reason.
 * - Must process inference asynchronously, ie. it must not block the caller
 * - Must return RUNTIME_STATUS_SUCCESS if input is successfully enqueued.
 * - Must return appropriate error status codes on validation or enqueue failures.
 * - Must be thread-safe. It must be safe to call this function from multiple threads for the same model or different ones
 */
RuntimeStatus runtime_enqueue_input(int model_id, Tensors *input_tensors);

/**
 * @brief Retrieve output tensors from a specific model after inference.
 *
 * @param model_id The identifier of the model to retrieve output from (index in
 * the models array from runtime_load_models).
 * @param output_tensors Output parameter to receive the output tensors of the
 * inference process. Must not be NULL.
 * @return RuntimeStatus indicating success or failure. Returns
 * RUNTIME_STATUS_NO_OUTPUT_AVAILABLE if no output is ready.
 *
 * @note Caller responsibility:
 * - Must ensure the model_id corresponds to a successfully loaded model.
 * - Must provide a valid pointer to Tensors** (output_tensors must be a valid pointer to a Tensors structure).
 * - Upon successful return (RUNTIME_STATUS_SUCCESS), caller owns all memory in the returned Tensors structure.
 * - Must free all memory associated with output_tensors after use, including:
 *   - All arrays (names, data_types, ranks, shapes, data)
 *   - All string data in names array
 *   - All tensor data in data array
 *   - The Tensors structure itself
 * - Must handle RUNTIME_STATUS_NO_OUTPUT_AVAILABLE by retrying later or checking inference status.
 *
 * @note Runtime responsibility:
 * - Must validate model_id is within valid range and corresponds to a loaded model.
 * - Must check if output is available for the specified model.
 * - Must return RUNTIME_STATUS_NO_OUTPUT_AVAILABLE if no output is ready yet.
 * - Must allocate and populate a new Tensors structure with output data on success.
 * - Must allocate all arrays (names, data_types, ranks, shapes, data) and their contents.
 * - Must allocate memory for all string data in names array.
 * - Must allocate memory for all tensor data in data array.
 * - Must set output_tensors to point to the newly allocated Tensors structure.
 * - Must return RUNTIME_STATUS_SUCCESS when output is successfully retrieved.
 * - Must return appropriate error status codes on failures.
 * - Must not modify or free the output_tensors after returning it to the caller.
 */
RuntimeStatus runtime_retrieve_output(int model_id, Tensors **output_tensors);

/**
 * @brief Clean up all resources and shutdown the runtime environment.
 *
 * @return RuntimeStatus indicating success or failure.
 *
 * @note Caller responsibility:
 * - Must not call any other runtime functions after this function returns successfully.
 * - Must call runtime_init() again after cleanup before re-using the runtime.
 *
 * @note Runtime responsibility:
 * - Must clean up any pending input that hasn't been processed yet.
 * - Must clean up any pending output that hasn't been retrieved yet.
 * - Must unload all loaded models and free associated resources.
 * - Must free all runtime-allocated memory and resources.
 * - Must return RUNTIME_STATUS_SUCCESS if cleanup completes successfully.
 * - Must return appropriate error status codes on failures.
 * - Must be idempotent (safe to call multiple times).
 * - Must ensure runtime is in a state where runtime_init() can be called again.
 */
RuntimeStatus runtime_cleanup(void);

/**
 * @brief Get the error message from the last runtime error.
 *
 * @return The error message in a human-readable format, or NULL if no error has occurred.
 *
 * @note Caller responsibility:
 * - May call this function after any runtime function returns a non-success status.
 * - Must not modify or free the returned string.
 * - Must not store the pointer beyond the lifetime of the runtime (before runtime_cleanup()).
 * - The returned string is valid until the next runtime function call or until runtime_cleanup().
 *
 * @note Runtime responsibility:
 * - Must allocate and maintain the error message string.
 * - Must update the error message whenever a runtime function returns an error status.
 * - Must return a valid null-terminated string on error, or NULL if no error occurred.
 * - Must ensure the returned string remains valid until the next runtime function call.
 * - Must free the error message string during runtime_cleanup().
 * - Must handle cases where no error has occurred (return NULL).
 */
const char *runtime_get_error(void);

/**
 * @brief Get the version string of the runtime library.
 *
 * @return The version string of the runtime library. Never returns NULL.
 *
 * @note Caller responsibility:
 * - May call this function at any time, even before runtime_init().
 * - Must not modify or free the returned string.
 * - The returned string is valid for the entire lifetime of the runtime.
 *
 * @note Runtime responsibility:
 * - Must return a valid null-terminated SemVer-compliant version string (e.g., "1.2.3").
 * - Must never return NULL.
 * - Must allocate the version string statically or maintain it for the runtime lifetime.
 * - Must ensure the returned string is valid for the entire lifetime of the runtime.
 */
const char *runtime_get_version(void);

/**
 * @brief Get the name of the runtime library.
 *
 * @return The name string of the runtime library. Never returns NULL.
 *
 * @note Caller responsibility:
 * - May call this function at any time, even before runtime_init().
 * - Must not modify or free the returned string.
 * - The returned string is valid for the entire lifetime of the runtime.
 *
 * @note Runtime responsibility:
 * - Must return a valid null-terminated name string identifying the runtime implementation.
 * - Must never return NULL.
 * - Must allocate the name string statically or maintain it for the runtime lifetime.
 * - Must ensure the returned string remains valid until runtime_cleanup() is called.
 * - Should return a unique identifier for the runtime backend (e.g., "OAAX-CUDA", "OAAX-ROCm").
 */
const char *runtime_get_name(void);

#ifdef __cplusplus
}
#endif

#endif // OAAX_RUNTIME_INTERFACE_H
