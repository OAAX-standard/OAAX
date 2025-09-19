# 📜 OAAX Specification (version 1.0)

This document outlines the official specification for OAAX (Open AI Accelerator eXchange). It describes the structure, components, and interfaces that constitute a valid OAAX implementation, including the toolchain and runtime.

## Core Components

OAAX consists of two core components:

| Component          | Description                                                                                                   | Specification                          |
| ------------------ | ------------------------------------------------------------------------------------------------------------- | -------------------------------------- |
| **OAAX Toolchain** | Docker-based conversion pipeline that takes an ONNX model and outputs a binary targeting a specific XPU       | [Specs](#oaax-toolchain-specification) |
| **OAAX Runtime**   | A shared library (.so or .dll) that implements the OAAX runtime API and executes the binary of a specific XPU | [Specs](#oaax-runtime-specification)   |



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

This section defines the **Application Program Interface (API)** and the structural expectations for OAAX-compliant runtime libraries. All OAAX runtimes **must conform** to this interface to ensure interoperability with tooling and host-side components.

---

#### 1. Overview

The OAAX runtime interface is defined as a **pure C API**, facilitating dynamic linking and cross-language compatibility. Runtimes are expected to expose a predefined set of functions for:

* Runtime initialization and destruction
* Model loading
* Inference input/output
* Error and version reporting

The runtime is loaded dynamically by the host using mechanisms such as `dlopen`/`dlsym` (POSIX) or `LoadLibrary`/`GetProcAddress` (Windows).

---

### 2. Header Definition

The interface is defined in the following header file: [interface.h](https://github.com/OAAX-standard/OAAX/blob/main/Illustrative%20example/Runtime/include/interface.h)

This file contains all required definitions. No additional symbols may be exposed by the runtime unless explicitly allowed by future revisions.

---

### 3. Data Types

#### 3.1 `tensor_data_type`

The following enumeration defines the supported tensor element types:

```c
typedef enum tensor_data_type {
    DATA_TYPE_FLOAT   = 1,
    DATA_TYPE_UINT8   = 2,
    DATA_TYPE_INT8    = 3,
    DATA_TYPE_UINT16  = 4,
    DATA_TYPE_INT16   = 5,
    DATA_TYPE_INT32   = 6,
    DATA_TYPE_INT64   = 7,
    DATA_TYPE_STRING  = 8,
    DATA_TYPE_BOOL    = 9,
    DATA_TYPE_DOUBLE  = 11,
    DATA_TYPE_UINT32  = 12,
    DATA_TYPE_UINT64  = 13
} tensor_data_type;
```

---

#### 3.2 `tensors_struct`

A structure representing a collection of tensors, either for input or output.

```c
typedef struct tensors_struct {
    size_t num_tensors;           // Number of tensors
    char** names;                 // Tensor names
    tensor_data_type* data_types;// Data types
    size_t* ranks;                // Number of dimensions per tensor
    size_t** shapes;             // Shapes (dimensionality)
    void** data;                 // Pointers to raw tensor data
} tensors_struct;
```

* Each field must be properly allocated and consistent in length across all arrays.
* Tensor shape arrays (`shapes[i]`) must contain exactly `ranks[i]` elements.
* Tensor names are UTF-8 encoded C-strings and must be **unique** within the list.

---

### 4. Runtime Interface Functions

Each runtime must implement the following functions:

#### 4.1 `int runtime_initialization(void);`

Initializes the runtime. Must be called exactly once prior to any other function, unless `runtime_initialization_with_args` is used instead.

**Returns:**

* `0` on success
* Non-zero value on failure

---

#### 4.2 `int runtime_initialization_with_args(int length, const char **keys, const void **values);`

Initializes the runtime with key-value arguments.

**Semantics:**

* If this function is used, `runtime_initialization()` must not be called.
* Unknown keys must be silently ignored.
* All arguments must remain valid for the duration of this call.

**Returns:**

* `0` on success
* Non-zero value on failure

---

#### 4.3 `int runtime_model_loading(const char *file_path);`

Loads a compiled OAAX model from the specified file path.

**Constraints:**

* The path must point to a valid model file.
* Only one model may be loaded per runtime instance.

**Returns:**

* `0` on success
* Non-zero value on failure

---

#### 4.4 `int send_input(tensors_struct *input_tensors);`

Transfers input tensors to the runtime for inference.  
The runtime must support receiving multiple input tensors sequentially without requiring the output of the previous inference to be retrieved first. This design enables efficient asynchronous processing by allowing the host to queue multiple inputs for inference, thereby maximizing throughput and minimizing idle time.

**Semantics:**

* The runtime takes ownership of the `input_tensors` structure and its internal memory.
* The runtime is responsible for freeing all memory related to the inputs.
* **If this function fails, the caller is responsible for memory cleanup.**
* The caller must ensure that the `input_tensors` structure is valid, pre-processed and properly populated before calling this function.

**Returns:**

* `0` on success
* Non-zero value on failure

---

#### 4.5 `int receive_output(tensors_struct **output_tensors);`

Used to retrieve output tensors from the runtime after inference if there are any.

**Semantics:**

* The runtime allocates and returns a `tensors_struct` via the provided pointer.
* The caller is responsible for freeing all memory associated with the output.

**Returns:**

* `0` if output was successfully retrieved
* Non-zero if no output is available or an error occurred

---

#### 4.6 `int runtime_destruction(void);`

Finalizes and releases all internal runtime resources.

**Returns:**

* `0` on success
* Non-zero value on failure

---

#### 4.7 `const char *runtime_error_message(void);`

Returns a pointer to a human-readable error string describing the last error.

**Ownership:**

* The returned string must be owned and freed (if necessary) by the runtime.

---

#### 4.8 `const char *runtime_version(void);`

Returns a static or heap-allocated string indicating the version of the runtime.

---

#### 4.9 `const char *runtime_name(void);`

Returns a static or heap-allocated string indicating the name of the runtime.

---

### 5. Memory Management Rules

* The host is responsible for freeing:

  * Any `tensors_struct*` returned by `receive_output`.
* The runtime is responsible for freeing:

  * All memory associated with input tensors passed to `send_input`, except when the function fails.
  * All memory internally allocated by the runtime during initialization, model loading, and inference.

---

### 6. Host Responsibilities

* Load the runtime shared library.
* Call one initialization function.
* Load the model before inference.
* Prepare and pass valid `tensors_struct` input via `send_input`.
* Retrieve and free outputs from `receive_output`.
* Finalize the runtime with `runtime_destruction`.

---

### 7. Runtime Responsibilities

* Properly allocate and free internal resources.
* Validate and store input tensors.
* Produce valid outputs and expose them via `receive_output`.
* Clean up upon destruction.
* Provide informative error and version strings.
* Maintain API compatibility as defined in this document.

 
---

## Change Process

Proposals to extend or modify the OAAX specification must be submitted via:

- GitHub pull requests or issues to [OAAX-standard/OAAX](https://github.com/OAAX-standard/OAAX)
