> **📦 Implementation details**  
> You can find the implementation of the CPU runtime & toolchain in the [Github repository](https://github.com/OAAX-standard/reference-implementation){:target="_blank"}.

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

The runtime library implements the OAAX's runtime interface for initializing, loading models, running inference and cleaning up the runtime.
It is initialized by calling `RuntimeStatus runtime_init(Config config);`, with the following parameters passed as string key/value pairs in `Config`:

- `log_level` (string, default is "2"): The minimal log level for the runtime. This can be set to `0` for trace, `1` for debug, `2` for info, `3` for warnings, `4` for errors, `5` for critical and `6` to disable logging.
- `log_file` (string, default is "runtime.log"): The file to which the runtime logs will be written.
- `num_threads` (string, from "1" to "16", default is "4"): The number of threads used by ONNX Runtime for inference. The higher the number, the more CPU resources will be used, but the better the throughput. It can also be set per model in `ModelConfig.config`.

> You can check out the examples repository for more details on how to use the runtime library: [OAAX Examples](https://github.com/OAAX-standard/examples).

### Conversion Toolchain

The conversion toolchain is used to validate, optimize and simplify the ONNX models. At the end of the process it produces a simplified ONNX model.

It can be used as follows (the container runs as a non-root user, so pass your own user to let it write to the mounted directory):
```bash
docker load -i oaax-cpu-toolchain.tar
docker run --rm --user $(id -u):$(id -g) -v ./model:/model oaax-cpu-toolchain:2.0.0 /model/model.onnx /model/output
```
The above command assumes that the model is located at `./model/model.onnx`.  
After a successful conversion, the generated model (`model-simplified.onnx`) and a conversion log (`logs.json`) are saved in the `./model/output` directory.

## Download links and compatibility matrix

Runtime libraries are packaged as `library.tar.gz` and contain `libRuntimeLibrary.so` (`RuntimeLibrary.dll` and the Visual C++ runtime DLLs on Windows).

### OAAX 2.0

| OAAX version | OS name | OS version      | CPU architecture | Runtime library | Conversion toolchain |
| ------------ | ------- | --------------- | ---------------- | --------------- | -------------------- |
| 2.0.0        | Ubuntu  | 20.04 or higher | x86_64           | [Download](https://nbg1.your-objectstorage.com/oaax/runtimes/2.0.0/CPU/x86_64/Ubuntu/library.tar.gz) | [Download](https://nbg1.your-objectstorage.com/oaax/conversion-toolchain/2.0.0/CPU/oaax-cpu-toolchain.tar) |
| 2.0.0        | Ubuntu  | 20.04 or higher | aarch64          | [Download](https://nbg1.your-objectstorage.com/oaax/runtimes/2.0.0/CPU/aarch64/Ubuntu/library.tar.gz) | ⬆️ |
| 2.0.0        | Windows | 10, 11          | x86_64           | [Download](https://nbg1.your-objectstorage.com/oaax/runtimes/2.0.0/CPU/x86_64/Windows/library.tar.gz) | ⬆️ |

### OAAX 1.x (legacy)

> **Note:** these builds implement the 1.x runtime API, which is not compatible with OAAX 2.0 hosts.

| OAAX version | OS name | OS version      | CPU architecture | Runtime library                                                                                       | Conversion toolchain                                                                                       |
| ------------ | ------- | --------------- | ---------------- | ----------------------------------------------------------------------------------------------------- | ---------------------------------------------------------------------------------------------------------- |
| 1.1.2        | Ubuntu  | 20.04 or higher | x86_64           | [Download](https://nbg1.your-objectstorage.com/oaax/runtimes/1.1.2/CPU/x86_64/Ubuntu/library.tar.gz)  | [Download](https://nbg1.your-objectstorage.com/oaax/conversion-toolchain/1.1.2/CPU/oaax-cpu-toolchain.tar) |
| 1.1.2        | Ubuntu  | 20.04 or higher | aarch64          | [Download](https://nbg1.your-objectstorage.com/oaax/runtimes/1.1.2/CPU/aarch64/Ubuntu/library.tar.gz) | ⬆️                                                                                                          |
| 1.1.2        | Windows | 10, 11          | x86_64           | [Download](https://nbg1.your-objectstorage.com/oaax/runtimes/1.1.2/CPU/x86_64/Windows/library.tar.gz) | ⬆️                                                                                                          |
