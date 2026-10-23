> **📦 Implementation details**  
> You can find the implementation of the Intel CPUs, GPUs and NPUs runtime & toolchain in the [Github repository](https://github.com/OAAX-standard/intel-acceleration){:target="_blank"}.

## Overview

OAAX provides a runtime and conversion toolchain for running ONNX models on Intel CPUs, GPUs and NPUs on both **Ubuntu** and **Windows**.

It uses the ONNX runtime and OpenVINO™ toolkit under the hood to offload the computation to Intel hardware.  
For more information about OpenVINO please visit the [OpenVINO website](https://docs.openvino.ai/2025/index.html).

## Requirements

### Hardware

Please consult the [OpenVINO documentation](https://docs.openvino.ai/2025/about-openvino/release-notes-openvino/system-requirements.html) for the most up-to-date hardware requirements.

### Operating System

- Debian 12 or higher
- Ubuntu 22.04 or higher
- Windows 10, 11

## Installation

To be able to use the Intel GPU and NPU, please make sure to install the latest drivers from Intel:

- For GPU, please follow the instructions [here](https://docs.openvino.ai/2025/get-started/install-openvino/configurations/configurations-intel-gpu.html){:target="_blank"}.
- For NPU, please follow the instructions [here](https://docs.openvino.ai/2025/get-started/install-openvino/configurations/configurations-intel-npu.html){:target="_blank"}.

## Usage

### Runtime Library

The runtime library implements the OAAX's runtime interface for initializing, loading models, running inference and cleaning up the runtime.
It is initialized by calling `RuntimeStatus runtime_init(Config config);`, with the following parameters passed as string key/value pairs in `Config`:

- `device_type` (string, default is "CPU"): The device used for inference. This can be set to `CPU`, `GPU`, or `NPU`. `GPU` activates MULTI automatically when several GPUs are found.
- `perf_hint` (string, default is "latency"): The OpenVINO performance mode. This can be set to `latency`, `throughput` or `cumulative_throughput`.
- `cache_dir` (string, default is "."): The directory of the OpenVINO compiled-model cache, which avoids recompiling the models at every start. Set to an empty string to disable the cache.
- `log_level` (string, default is "2"): The minimal log level for the runtime. This can be set to `0` for trace, `1` for debug, `2` for info, `3` for warnings, `4` for errors, `5` for critical and `6` to disable logging.
- `log_file` (string, default is "runtime.log"): The file to which the runtime logs will be written.

`device_type`, `perf_hint` and `cache_dir` can also be set per model in `ModelConfig.config`, which overrides the global value, e.g. to run two models on different devices. Models are loaded from the `.zip` file produced by the conversion toolchain.

> You can check out the examples repository for more details on how to use the runtime library: [OAAX Examples](https://github.com/OAAX-standard/examples).

### Conversion Toolchain

The conversion toolchain converts an ONNX model to the OpenVINO IR format. By default, it compresses the weights to FP16.

The input is a `.zip` bundle containing `model.onnx` and, optionally, a `config.json` and a `calibration/` directory of images. The container runs as a non-root user, so pass your own user to let it write to the mounted directories:
```bash
docker load -i oaax-intel-toolchain.tar   # loads the image as oaax-intel-toolchain:latest
mkdir -p input output
zip input/model.zip model.onnx
docker run --rm --user $(id -u):$(id -g) -v ./input:/input -v ./output:/output oaax-intel-toolchain:latest /input/model.zip /output
```
After a successful conversion, the converted model (`model.zip`, containing `model.xml` and `model.bin`) and a conversion log (`logs.json`) are saved in the `./output` directory. Load `model.zip` with the runtime.

With the default FP16 compression, the runtime expects FP16 input tensors. To keep the model's original input types (e.g. FP32), disable it with a `config.json` in the bundle:
```json
{ "optimization": { "fp16_compression": false } }
```
```bash
zip input/model.zip model.onnx config.json
```
INT8 quantization is also available (`"quantization": {"enabled": true, "preset": "mixed", "subset_size": 300}`, with calibration images in `calibration/`); see the [toolchain README](https://github.com/OAAX-standard/intel-acceleration/blob/main/conversion-toolchain/README.md).

## Download links and compatibility matrix

Runtime libraries are packaged as `library.tar.gz`. Extract all the files into the same directory: the runtime finds the OpenVINO libraries next to itself.

### OAAX 2.0

| OAAX version | OS      | Version      | CPU architecture | Runtime library | Conversion toolchain |
| ------------ | ------- | ------------ | ---------------- | --------------- | -------------------- |
| 2.0.0        | Ubuntu  | 22.04 or higher | x86_64        | [Download](https://nbg1.your-objectstorage.com/oaax/runtimes/2.0.0/INTEL/x86_64/Ubuntu/library.tar.gz) | [Download](https://nbg1.your-objectstorage.com/oaax/conversion-toolchain/2.0.0/INTEL/oaax-intel-toolchain.tar) |
| 2.0.0        | Windows | 10, 11       | x86_64           | [Download](https://nbg1.your-objectstorage.com/oaax/runtimes/2.0.0/INTEL/x86_64/Windows/library.tar.gz) | ⬆️ |

### OAAX 1.x (legacy)

> **Note:** these builds implement the 1.x runtime API, which is not compatible with OAAX 2.0 hosts.

| OAAX version | OS      | Version             | CPU architecture | Runtime library                                                                                         | Conversion toolchain                                                                                           |
| ------------- | ------- | ------------------- | ---------------- | ------------------------------------------------------------------------------------------------------- | -------------------------------------------------------------------------------------------------------------- |
| 1.1.1         | Ubuntu  | 22.04, 24.04 | x86_64           | [Download](https://nbg1.your-objectstorage.com/oaax/runtimes/1.1.1/INTEL/x86_64/Ubuntu/library.tar.gz)  | [Download](https://nbg1.your-objectstorage.com/oaax/conversion-toolchain/1.1.1/INTEL/oaax-intel-toolchain.tar) |
| 1.1.1         | Windows | 10, 11              | x86_64           | [Download](https://nbg1.your-objectstorage.com/oaax/runtimes/1.1.1/INTEL/x86_64/Windows/library.tar.gz) | [Download](https://nbg1.your-objectstorage.com/oaax/conversion-toolchain/1.1.1/INTEL/oaax-intel-toolchain.tar) |
