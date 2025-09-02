# Overview

OAAX provides a runtime and conversion toolchain for running ONNX models on CPUs on both **Ubuntu** and **Windows**.

# Requirements

## Hardware

- x86_64 or aarch64 CPU
- AVX2 instruction set support (for x86_64)

## Software

- Debian 11 or higher 
- Ubuntu 20.04 or higher
- Windows 10, 11 (for x86_64)

# Usage

## Runtime Library

The runtime library implements the OAAX's runtime interface for initializing, loading, running inference and destroying the runtime.
The initialization in particular can be done without providing a configuration by calling `int runtime_initialization();` directly or by providing these parameters to `int runtime_initialization_with_args(int length, char **keys, void **values);`:

- `n_duplicates`: Number of duplicates of the model to create. This is useful for running multiple inferences asynchronously. Default value is 1.
- `n_threads_per_duplicate`: Number of threads to use per duplicate. This helps control the amount of CPU resources to dedicate to the inference. Default value is 4.
- `queue_capacity`: The maximum number of inference requests that can be queued. This is useful for controlling the memory usage and performance. Default value is 100.
- `runtime_log_level`: The minimal log level for the runtime. This can be set to `0` for debug, `1` for info, `2` for warnings, and `3` for errors. The default is 1 to log info, warnings and errors.

> You can check out the examples repository for more details on how to use the runtime library: [OAAX Examples](https://github.com/OAAX-standard/examples).

## Conversion Toolchain

The conversion toolchain is used to validate, optimize and simplify the ONNX models. At the end of the process it produces a simplified ONNX model.

It can be used as follows:
```bash
docker run -v /path/to/model-directory:/model  oaax-cpu-toolchain:latest /model/model.onnx /model/output
```

# Download links and compatibility matrix

| OAAX versions | OS      | Version             | CPU architecture | Runtime library                                                                                       | Conversion toolchain                                                                                               |
| ------------- | ------- | ------------------- | ---------------- | ----------------------------------------------------------------------------------------------------- | ------------------------------------------------------------------------------------------------------------------ |
| 1.1.0         | Ubuntu  | 20.04, 22.04, 24.04 | x86_64           | [Download](https://nbg1.your-objectstorage.com/oaax/runtimes/1.1.0/CPU/Ubuntu/x86_64/library.tar.gz)  | [Download](https://nbg1.your-objectstorage.com/oaax/conversion-toolchain/1.1.0/CPU/oaax-cpu-toolchain.tar)         |
| 1.1.0         | Ubuntu  | 20.04, 22.04, 24.04 | aarch64          | [Download](https://nbg1.your-objectstorage.com/oaax/runtimes/1.1.0/CPU/Ubuntu/aarch64/library.tar.gz) | [Download](https://nbg1.your-objectstorage.com/oaax/conversion-toolchain/1.1.0/CPU/oaax-cpu-toolchain-aarch64.tar) |
| 1.1.0         | Windows | 10, 11              | x86_64           | [Download](https://nbg1.your-objectstorage.com/oaax/runtimes/1.1.0/CPU/Windows/x86_64/library.tar.gz) | [Download](https://nbg1.your-objectstorage.com/oaax/conversion-toolchain/1.1.0/CPU/oaax-cpu-toolchain.zip)         |
