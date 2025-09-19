## Overview

OAAX provides a runtime and conversion toolchain for running ONNX models on Intel CPUs, GPUs and NPUs on both **Ubuntu** and **Windows**.

It uses the ONNX runtime and OpenVINO™ toolkit under the hood to offload the computation to Intel hardware.  
For more information about OpenVINO please visit the [OpenVINO website](https://docs.openvino.ai/2025/index.html).

## Requirements

### Hardware

Please consult the [OpenVINO documentation](https://docs.openvino.ai/2025/about-openvino/release-notes-openvino/system-requirements.html) for the most up-to-date hardware requirements.

### Operating System

- Debian 11 or higher
- Ubuntu 20.04 or higher
- Windows 10, 11

## Installation

To be able to use the Intel GPU and NPU, please make sure to install the latest drivers from Intel:

- For GPU, please follow the instructions [here](https://docs.openvino.ai/2025/get-started/install-openvino/configurations/configurations-intel-gpu.html){:target="_blank"}.
- For NPU, please follow the instructions [here](https://docs.openvino.ai/2025/get-started/install-openvino/configurations/configurations-intel-npu.html){:target="_blank"}.

## Usage

### Runtime Library

The runtime library implements the OAAX's runtime interface for initializing, loading, running inference and destroying the runtime.
The initialization in particular can be done without providing a configuration by calling `int runtime_initialization();` directly or by providing these parameters to `int runtime_initialization_with_args(int length, char **keys, void **values);`:

- `log_level` (char *, default is "2"): The minimal log level for the runtime. This can be set to `0` for trace, `1` for debug, `2` for info, `3` for warnings, `4` for errors, `5` for critical and `6` to disable logging.
- `log_file` (char *, default is "runtime.log"): The file to which the runtime logs will be written. If not specified, logs will be written to stdout.
- `num_threads` (char *, default is "8"): The maximum number of threads that can be used by the runtime. The higher the number, the more CPU resources will be used, but the better the throughput.
- `device_type` (char *, default is "CPU"): The type of device to use for inference. This can be set to `CPU`, `GPU`, or `NPU`. The default is `CPU`.
- `precision` (char *, default is "FP32"): The precision to use for inference. This can be set to `FP32`, `FP16`, or `ACCURACY`. The default is `FP32`.

> You can check out the examples repository for more details on how to use the runtime library: [OAAX Examples](https://github.com/OAAX-standard/examples).

### Conversion Toolchain

The conversion toolchain is used to validate, optimize and simplify the ONNX models. At the end of the process it produces a simplified ONNX model.

It can be used as follows:
```bash
docker run -v ./model:/model  oaax-intel-toolchain:1.1.1 /model/model.onnx /model/output
```
The above command assumes that the model is located at `./model/model.onnx`.  
After a succesful conversion, the generated model will be saved in the `./model/output` directory.


## Download links and compatibility matrix

| OAAX versions | OS      | Version             | CPU architecture | Runtime library                                                                                         | Conversion toolchain                                                                                           |
| ------------- | ------- | ------------------- | ---------------- | ------------------------------------------------------------------------------------------------------- | -------------------------------------------------------------------------------------------------------------- |
| 1.1.1         | Ubuntu  | 22.04, 24.04 | x86_64           | [Download](https://nbg1.your-objectstorage.com/oaax/runtimes/1.1.1/INTEL/x86_64/Ubuntu/library.tar.gz)  | [Download](https://nbg1.your-objectstorage.com/oaax/conversion-toolchain/1.1.0/INTEL/oaax-intel-toolchain.tar) |
| 1.1.1         | Windows | 10, 11              | x86_64           | [Download](https://nbg1.your-objectstorage.com/oaax/runtimes/1.1.1/INTEL/x86_64/Windows/library.tar.gz) | [Download](https://nbg1.your-objectstorage.com/oaax/conversion-toolchain/1.1.0/INTEL/oaax-intel-toolchain.tar) |
