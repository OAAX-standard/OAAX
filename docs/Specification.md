# 📜 OAAX Specification (version 2.0)

This document outlines the official specification for OAAX (Open AI Accelerator eXchange). It describes the structure, components, and interfaces that constitute a valid OAAX implementation, including the toolchain and runtime.

## Core Components

OAAX consists of two core components:

| Component          | Description                                                                                                   | Specification                          |
| ------------------ | ------------------------------------------------------------------------------------------------------------- | -------------------------------------- |
| **OAAX Toolchain** | Docker-based conversion pipeline that takes an ONNX model and outputs a binary targeting a specific XPU       | [Specs](#oaax-toolchain-specification) |
| **OAAX Runtime**   | A shared library (.so or .dll) that implements the OAAX runtime C API and executes the binary of a specific XPU | [Specs](#oaax-runtime-specification)   |



## OAAX Toolchain Specification

This section defines the **requirements and interface** for OAAX-compliant toolchain implementations. All OAAX toolchains **must conform** to this specification to ensure compatibility and interoperability within the OAAX ecosystem.

---

### 1. Overview

The OAAX toolchain is a **Docker-based conversion pipeline** responsible for transforming hardware-agnostic AI models (in ONNX format) into XPU-specific optimized formats. The toolchain encapsulates all necessary dependencies, tools, and conversion logic within a containerized environment, ensuring:

* **Portability**: Cross-platform execution capability
* **Isolation**: Clean, reproducible conversion environments
* **Flexibility**: Freedom for XPU manufacturers to use proprietary conversion tools
* **Standardization**: Uniform interface regardless of underlying implementation

The toolchain formally implements the conversion function: `f : M → M_xpu`, where `M` represents the generic ONNX model, and `M_xpu` represents the XPU-specific optimized model.

---

### 2. Docker Container Requirements

#### 2.1 Container Structure

Every OAAX toolchain must be packaged as a Docker image with the following characteristics:

* **Working Directory**: `/app` (recommended)
* **Entry Point**: Must accept exactly two command-line arguments
* **Network Access**: Container should not require network connectivity
* **User Permissions**: Must not require root privileges for normal operation

#### 2.2 Entry Point Specification

The Docker container's entry point must implement the following signature:

```bash
<toolchain-entrypoint> <input-path> <output-directory>
```

**Parameters:**
* `<input-path>`: Absolute path to the input model file or archive within the container
* `<output-directory>`: Absolute path to the directory where conversion outputs will be written. If the directory does not exist, it must be created by the toolchain.

**Example Usage:**
```bash
docker run -v "$PWD:/app/run" oaax-toolchain-vendor "/app/run/model.onnx" "/app/run/build"
```

---

### 3. Input Requirements

#### 3.1 Supported Input Formats

OAAX toolchains must support at least one of the following input formats:

**Primary Format:**
* **ONNX Model File** (`.onnx`): Standard ONNX model representation

**Extended Format (Optional):**
* **Archive File** (`.zip`, `.tar.gz`): Containing ONNX model plus supplementary files such as:
  * Calibration datasets for quantization
  * Configuration files
  * Validation datasets

#### 3.2 Input Validation

Toolchains must perform input validation and provide meaningful error messages for:

* **File Format Validation**: Verify ONNX model integrity
* **Operator Support**: Check if all required operators are supported
* **Model Architecture**: Validate compatibility with target XPU capabilities
* **Input Constraints**: Verify model meets XPU-specific limitations (memory, precision, etc.)

---

### 4. Conversion Process

Toolchains should provide progress information through:

* **Console Output**: Real-time conversion status messages
* **Error Reporting**: Human-readable error messages with actionable guidance

---

### 5. Output Requirements

#### 5.1 Mandatory Outputs

Every successful toolchain execution must produce:

**1. Model File**: XPU-specific model representation
* **Filename**: Flexible naming (e.g., `model.oaax`, `model.bin`, `optimized_model.zip`)
* **Validation**: Must be loadable by corresponding OAAX runtime
* **I/O Compatibility**: Must retain input/output tensor names and shapes of the original ONNX model

**2. Conversion Log**: JSON-formatted processing log
* **Filename**: `conversion-log.json` or `logs.json`
* **Content**: Structured information about the conversion process

#### 5.2 Output Validation

Toolchains must ensure:

* **File Integrity**: All output files are complete and valid
* **Metadata Accuracy**: Log information correctly reflects actual conversion results

---

### 6. Error Handling

#### 6.1 Exit Codes

Toolchains must use standard exit codes:

* **0**: Successful conversion,
* **Non-zero**: otherwise.

#### 6.2 Error Reporting

For all failure cases, toolchains must:

* **Generate Logs**: Produce conversion-log.json with error details
* **Provide Context**: Include actionable error messages
* **Maintain Consistency**: Use standardized error codes and categories

---

### 7. Container Distribution

#### 7.1 Image Naming Convention

OAAX toolchain containers should follow naming conventions:

```
oaax-<vendor>-toolchain[-<version>]:tag
```

**Examples:**
* `oaax-nvidia-toolchain-v2.1:latest`
* `oaax-intel-openvino-toolchain:1.0.0`


---

### 8. Compliance Validation

#### 8.1 Toolchain Requirements Checklist

An OAAX-compliant toolchain must:

- [ ] Accept exactly two command-line arguments (input path, output directory)
- [ ] Process valid ONNX models successfully
- [ ] Generate both model file and conversion log on success
- [ ] Use appropriate exit codes for different failure types
- [ ] Provide meaningful error messages in conversion logs
- [ ] Handle file permissions correctly across platforms
- [ ] Not require network connectivity
- [ ] Preferably, execute without root privileges

#### 8.2 Testing and Validation

Toolchain developers should validate their implementation using:

* **Standard Test Models**: Common ONNX architectures (ResNet, YOLOv5, etc.)
* **Edge Cases**: Invalid inputs, unsupported operators, resource constraints
* **Integration Testing**: Compatibility with corresponding OAAX runtime
* **Cross-Platform Testing**: Verification on different host operating systems

---

### 9. Best Practices

#### 9.1 Performance Optimization

* **Parallel Processing**: Utilize multi-core capabilities where possible
* **Memory Management**: Efficient memory usage for large models
* **Caching**: Cache intermediate results for iterative conversions
* **Progress Reporting**: Provide conversion progress feedback for long operations

#### 9.2 User Experience

* **Clear Documentation**: Provide comprehensive usage documentation
* **Verbose Logging**: Include debug-level logging options
* **Model Compatibility**: Clearly document supported model architectures
* **Resource Requirements**: Specify minimum system requirements

#### 9.3 Security Considerations

* **Input Sanitization**: Validate all inputs to prevent security vulnerabilities
* **Resource Limits**: Implement reasonable memory and CPU usage limits
* **File System Access**: Restrict file system access to mounted volumes only

---

## OAAX Runtime Specification

This section defines the **C API** that OAAX-compliant runtime libraries must implement. The header [`interface.h`](https://github.com/OAAX-standard/OAAX/blob/main/include/interface.h) is **normative**: it carries the full caller/runtime contract of every function, and this page only summarizes it. If the two ever disagree, the header wins.

---

### 1. Overview

A runtime is a shared library (`.so` or `.dll`) exposing a **pure C API**. The host loads it dynamically (`dlopen`/`dlsym` on POSIX, `LoadLibrary`/`GetProcAddress` on Windows).

The API is asynchronous: inputs are enqueued without blocking, and outputs are retrieved separately. One runtime can hold **several models**, each addressed by its index (`model_id`) in the array passed to `runtime_load_models`.

---

### 2. Lifecycle

```
runtime_init → runtime_load_models → runtime_enqueue_input / runtime_retrieve_output → runtime_cleanup
```

1. `runtime_init(Config)`: prepares the runtime. Must not be called twice without `runtime_cleanup()` in between.
2. `runtime_load_models(n, configs)`: loads `n` models, from a file path or from memory.
3. Any number of `runtime_enqueue_input` and `runtime_retrieve_output` calls.
4. `runtime_cleanup()`: drops pending inputs/outputs, unloads the models and frees everything. `runtime_init` can be called again afterwards.

`runtime_get_version` and `runtime_get_name` can be called at any time, even before `runtime_init`. `runtime_get_error` can be called after any function returned a non-success status. `runtime_get_info` requires an initialized runtime.

---

### 3. Data types

#### 3.1 `RuntimeStatus`

Returned by every function except the getters. `RUNTIME_STATUS_SUCCESS` is `0`.

| Value | Name                                   | Value | Name                                |
| ----- | -------------------------------------- | ----- | ----------------------------------- |
| 0     | `RUNTIME_STATUS_SUCCESS`               | 10    | `RUNTIME_STATUS_TENSOR_SHAPE_MISMATCH` |
| 1     | `RUNTIME_STATUS_ERROR`                 | 11    | `RUNTIME_STATUS_TENSOR_TYPE_MISMATCH`  |
| 2     | `RUNTIME_STATUS_NOT_INITIALIZED`       | 12    | `RUNTIME_STATUS_NO_OUTPUT_AVAILABLE`   |
| 3     | `RUNTIME_STATUS_ALREADY_INITIALIZED`   | 13    | `RUNTIME_STATUS_INFERENCE_ERROR`       |
| 4     | `RUNTIME_STATUS_MODEL_NOT_LOADED`      | 14    | `RUNTIME_STATUS_NOT_IMPLEMENTED`       |
| 5     | `RUNTIME_STATUS_INVALID_ARGUMENT`      | 15    | `RUNTIME_STATUS_TIMEOUT`               |
| 6     | `RUNTIME_STATUS_INVALID_MODEL`         | 16    | `RUNTIME_STATUS_DEVICE_ERROR`          |
| 7     | `RUNTIME_STATUS_FILE_NOT_FOUND`        | 17    | `RUNTIME_STATUS_UNKNOWN_ERROR`         |
| 8     | `RUNTIME_STATUS_OUT_OF_MEMORY`         | 18    | `RUNTIME_STATUS_INVALID_MODEL_ID`      |
| 9     | `RUNTIME_STATUS_INVALID_TENSOR`        |       |                                     |

The header gives explicit semantics for `ALREADY_INITIALIZED` (second `runtime_init` without cleanup) and `NO_OUTPUT_AVAILABLE` (no output ready). After any non-success status, `runtime_get_error()` returns a human-readable message.

#### 3.2 `TensorElementType`

Element type of a tensor. The values are the **ONNX data type enum** (`DATA_TYPE_UNDEFINED = 0`, `DATA_TYPE_FLOAT = 1`, `DATA_TYPE_UINT8 = 2`, ..., `DATA_TYPE_INT2 = 26`); see the header for the full list.

#### 3.3 `TensorDescriptor` and `Tensors`

```c
typedef struct TensorDescriptor {
  char              *name;      // Null-terminated tensor name
  TensorElementType  data_type; // Element type
  int                rank;      // Number of dimensions
  int               *shape;     // Array of `rank` dimension sizes
  size_t             data_size; // Size of `data` in bytes
  void              *data;      // Tensor data buffer
} TensorDescriptor;

typedef struct Tensors {
  int               id;          // Caller-assigned request ID, echoed on the output
  int               num_tensors; // Number of tensors
  TensorDescriptor *tensors;     // Array of num_tensors descriptors
} Tensors;
```

* `tensors` is a single allocation of `num_tensors` descriptors. Each descriptor's `name`, `shape` and `data` are allocated separately.
* `id` identifies a request. The caller sets it on each input; the runtime copies it unchanged onto the matching output and never interprets it. This is how the host matches outputs to requests when several are in flight.

#### 3.4 `Config` and `ModelConfig`

```c
typedef struct Config {
  int          length; // Number of key/value pairs
  const char **keys;
  const char **values; // Always null-terminated strings
} Config;

typedef struct ModelConfig {
  const char          *file_path;  // Path to the model file (optional)
  const unsigned char *model_data; // Model in memory (optional)
  size_t               model_size; // Size of model_data (optional)
  Config               config;     // Model-specific configuration
} ModelConfig;
```

Each `ModelConfig` provides either `file_path` or `model_data` + `model_size`. All configuration values are strings (e.g. `"2"`, `"latency"`, `"/tmp/cache"`). The keys a runtime understands are vendor-specific, and unknown keys must be ignored.

---

### 4. Functions

| Function | Summary |
| -------- | ------- |
| `RuntimeStatus runtime_init(Config config)` | Initializes the runtime. Unknown config keys are ignored. Returns `ALREADY_INITIALIZED` (state untouched) if already initialized. On failure the runtime stays uninitialized and the caller may retry. |
| `RuntimeStatus runtime_load_models(int num_models, const ModelConfig *model_configs)` | Loads all models; model `i` gets `model_id = i`. Requires a successful `runtime_init`. If any model fails, everything allocated by the call is freed and the caller may retry with another set of models. Nothing passed in (array, paths, buffers, config) may be assumed valid after the call returns: the runtime copies what it needs. |
| `RuntimeStatus runtime_enqueue_input(int model_id, Tensors *input_tensors)` | Enqueues one inference request for a model. Asynchronous: must not block the caller. Validates `model_id`, the tensors, and that shapes and types match the model's inputs. |
| `RuntimeStatus runtime_retrieve_output(int *model_id, Tensors **output_tensors, int timeout_ms)` | Retrieves the output of any model's inference. `*model_id` receives the model it belongs to, `*output_tensors` the newly allocated result. Both pointers must be non-`NULL`. |
| `RuntimeStatus runtime_cleanup(void)` | Shuts the runtime down (see [Lifecycle](#2-lifecycle)). Idempotent. |
| `const char *runtime_get_error(void)` | Message of the last error, or `NULL` if none. Valid until the next runtime call or `runtime_cleanup()`. |
| `const char *runtime_get_version(void)` | SemVer version string of the runtime. Never `NULL`. Callable before `runtime_init`. |
| `const char *runtime_get_name(void)` | Name identifying the runtime backend (e.g. `"OAAX-CUDA"`). Never `NULL`. Callable before `runtime_init`. |
| `const char *runtime_get_info(void)` | JSON object describing the runtime state (e.g. `active_device`, `loaded_models`, `requests_in_flight`). Returns `NULL` if the runtime is not initialized. All fields are optional and the caller must ignore unknown ones. Valid until the next runtime call or `runtime_cleanup()`. |

#### 4.1 `runtime_retrieve_output` timeout

* `timeout_ms == 0`: returns immediately if no output is available.
* `timeout_ms < 0`: waits indefinitely until an output is available.
* `timeout_ms > 0`: waits up to that many milliseconds.

`RUNTIME_STATUS_NO_OUTPUT_AVAILABLE` is returned if no output is ready.

---

### 5. Memory ownership

| Memory | Allocated by | Freed by |
| ------ | ------------ | -------- |
| Input `Tensors` passed to `runtime_enqueue_input` | Host | **Runtime** after inference completes, if the call returned `SUCCESS`. **Host** if the call failed. |
| Output `Tensors` from `runtime_retrieve_output` | Runtime | **Host**, after a `SUCCESS` return. |
| Strings returned by getters | Runtime | Nobody: the host must not modify or free them. |
| `Config`, `ModelConfig` and what they point to | Host | Host (the runtime only borrows them for the duration of the call). |

To deep-free a `Tensors`: free each descriptor's `name`, `shape` and `data`, then `tensors`, then the `Tensors` struct itself.

---

### 6. Concurrency

* `runtime_enqueue_input` is **thread-safe**: it may be called from several threads, for the same model or different ones.
* Several requests may be in flight at once. The host does not have to retrieve an output before enqueuing the next input.
* Outputs from all models are retrieved through the same `runtime_retrieve_output` call; use `*model_id` and `Tensors.id` to route them.

---

### 7. Responsibilities

**Host**

* Call `runtime_init` first, and `runtime_load_models` before enqueuing inputs.
* Set a unique `Tensors.id` on every input, and provide tensors whose data matches the declared types and shapes.
* Free every output received, and free inputs that were rejected.
* Call `runtime_cleanup` when done. No other runtime function may be called afterwards, until `runtime_init` is called again.

**Runtime**

* Validate `model_id`, tensors and configuration, and report problems with the appropriate `RuntimeStatus` and error message.
* Process inference asynchronously and never block `runtime_enqueue_input`.
* Echo `Tensors.id` on outputs, and allocate outputs entirely (struct, descriptors, names, shapes, data).
* Free everything it allocated on failure, and on `runtime_cleanup`.

---

## Change Process

Proposals to extend or modify the OAAX specification must be submitted via:

- GitHub pull requests or issues to [OAAX-standard/OAAX](https://github.com/OAAX-standard/OAAX)
