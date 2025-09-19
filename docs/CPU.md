## Overview

The CPU reference implementation is the baseline OAAX runtime and toolchain.
It demonstrates the OAAX standard in practice and provides a portable way to run ONNX models on x86_64 and aarch64 CPUs across Linux and Windows.

This page explains how to convert a model with the CPU toolchain and run it with the CPU runtime.

## Requirements

### Hardware

- x86_64 or aarch64 CPU
- AVX2 instruction set support (for x86_64)

### Software

- Debian 11 or higher 
- Ubuntu 20.04 or higher
- Windows 10, 11 (only when running on x86_64)

## Usage

### Runtime Library

The runtime library implements the OAAX's runtime interface for initializing, loading, running inference and destroying the runtime.
The initialization in particular can be done without providing parameters by calling `int runtime_initialization();` directly or by providing them to `int runtime_initialization_with_args(int length, char **keys, void **values);`.

The following is the list of supported parameters:

- `log_level` (char *, default is "2"): The minimal log level for the runtime. This can be set to `0` for trace, `1` for debug, `2` for info, `3` for warnings, `4` for errors, `5` for critical and `6` to disable logging.
- `log_file` (char *, default is "runtime.log"): The file to which the runtime logs will be written. If not specified, logs will be written to stdout.
- `num_threads` (char *, default is "4"): The maximum number of threads that can be used by the runtime. The higher the number, the more CPU resources will be used, but the better the throughput.

> You can check out the examples repository for more details on how to use the runtime library: [OAAX Examples](https://github.com/OAAX-standard/examples).

### Conversion Toolchain

The conversion toolchain is used to validate, optimize and simplify the ONNX models. At the end of the process it produces a simplified ONNX model.

It can be used as follows:
```bash
docker run -v ./model:/model  oaax-cpu-toolchain:latest /model/model.onnx /model/output
```
The above command assumes that the model is located at `./model/model.onnx`.  
After a succesful conversion, the generated model will be saved in the `./model/output` directory.

## Download links and compatibility matrix

| OAAX version | OS name | OS version      | CPU architecture | Runtime library                                                                                       | Conversion toolchain                                                                                       |
| ------------ | ------- | --------------- | ---------------- | ----------------------------------------------------------------------------------------------------- | ---------------------------------------------------------------------------------------------------------- |
| 1.1.2        | Ubuntu  | 20.04 or higher | x86_64           | [Download](https://nbg1.your-objectstorage.com/oaax/runtimes/1.1.2/CPU/x86_64/Ubuntu/library.tar.gz)  | [Download](https://nbg1.your-objectstorage.com/oaax/conversion-toolchain/1.1.2/CPU/oaax-cpu-toolchain.tar) |
| 1.1.2        | Ubuntu  | 20.04 or higher | aarch64          | [Download](https://nbg1.your-objectstorage.com/oaax/runtimes/1.1.2/CPU/aarch64/Ubuntu/library.tar.gz) | ⬆️                                                                                                          |
| 1.1.2        | Windows | 10, 11          | x86_64           | [Download](https://nbg1.your-objectstorage.com/oaax/runtimes/1.1.2/CPU/x86_64/Windows/library.tar.gz) | ⬆️                                                                                                          |
