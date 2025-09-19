## Overview

For NVIDIA GPUs, OAAX provides an implementation of conversion toolchain and runtime library. The latter is based on the ONNX Runtime with [CUDA Execution Provider](https://onnxruntime.ai/docs/execution-providers/CUDA-ExecutionProvider.html){:target="_blank"}.

## Requirements

Most NVIDIA GPUs are supported, including:

- Data Center / Desktop GPUs with compute capability 3.5 or higher (e.g., T4, A10, A100, RTX 3000 series, RTX 4000 series) running on Ubuntu 20.04 or higher or Windows 10/11.
- NVIDIA Jetson series (e.g. TX2, Xavier NX, AGX Orin) running JetPack 5.0 or higher.

## Installation

To use the NVIDIA GPU acceleration, please ensure that you have the latest NVIDIA drivers and CUDA toolkit installed:
- For Data Center / Desktop GPUs, please follow the instructions [here](https://developer.nvidia.com/cuda-downloads){:target="_blank"}.
- For NVIDIA Jetson series, please follow the instructions [here](https://developer.nvidia.com/embedded/jetpack){:target="_blank"}.

## Usage

### Runtime Library

The runtime library implements the OAAX's runtime interface for initializing, loading, running inference and destroying the runtime.
The initialization in particular can be done without providing a configuration by calling `int runtime_initialization();` directly or by providing these parameters to `int runtime_initialization_with_args(int length, char **keys, void **values);`:

- `log_level` (char *, default is "2"): The minimal log level for the runtime. This can be set to `0` for trace, `1` for debug, `2` for info, `3` for warnings, `4` for errors, `5` for critical and `6` to disable logging.
- `log_file` (char *, default is "runtime.log"): The file to which the runtime logs will be written. If not specified, logs will be written to stdout.
- `num_threads` (char *, default is "4"): The maximum number of threads that can be used by the runtime. The higher the number, the more CPU resources will be used, but the better the throughput.

> You can check out the examples repository for more details on how to use the runtime library: [OAAX Examples](https://github.com/OAAX-standard/examples).

### Conversion Toolchain

The conversion toolchain is used to validate, optimize and simplify the ONNX models. At the end of the process it produces a simplified ONNX model.

It can be used as follows:
```bash
docker run -v ./model:/model  oaax-nvidia-toolchain:1.1.1 /model/model.onnx /model/output
```
The above command assumes that the model is located at `./model/model.onnx`.  
After a succesful conversion, the generated model will be saved in the `./model/output` directory.


## Download links and compatibility matrix

| OAAX versions | OS             | Version         | CPU architecture | Runtime library                                                                                                                                                                                                                      | Conversion toolchain                                                                                             |
| ------------- | -------------- | --------------- | ---------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ | ---------------------------------------------------------------------------------------------------------------- |
| 1.1.1         | Ubuntu         | 20.04 or higher | x86_64           | [CUDA 11](https://oaax.nbg1.your-objectstorage.com/runtimes/1.1.1/NVIDIA/x86_64/Ubuntu/library-cuda_11.tar.gz), [CUDA 12](https://oaax.nbg1.your-objectstorage.com/runtimes/1.1.1/NVIDIA/x86_64/Ubuntu/library-cuda_12.tar.gz)       | [Download](https://oaax.nbg1.your-objectstorage.com/conversion-toolchain/1.1.1/NVIDIA/oaax-nvidia-toolchain.tar) |
| 1.1.1         | NVIDIA JetPack | -               | ARM64            | [JetPack 5](https://oaax.nbg1.your-objectstorage.com/runtimes/1.1.1/NVIDIA/aarch64/Ubuntu/library-cuda_11.tar.gz), [JetPack 6](https://oaax.nbg1.your-objectstorage.com/runtimes/1.1.1/NVIDIA/aarch64/Ubuntu/library-cuda_12.tar.gz) | ⬆️                                                                                                                |
| 1.1.1         | Windows        | 10 or higher    | x86_64           | [CUDA 11](https://oaax.nbg1.your-objectstorage.com/runtimes/1.1.1/NVIDIA/x86_64/Windows/library-cuda_11.tar.gz), [CUDA 12](https://oaax.nbg1.your-objectstorage.com/runtimes/1.1.1/NVIDIA/x86_64/Windows/library-cuda_12.tar.gz)     | ⬆️                                                                                                                |