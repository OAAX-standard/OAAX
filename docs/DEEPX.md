> **📦 Implementation details**  
> You can find the implementation of the DEEPX NPUs runtime & toolchain in the [Github repository](https://github.com/OAAX-standard/deepx-acceleration){:target="_blank"}.

## Overview

DEEPX is a leading on-device AI semiconductor company specializing in Neural Processing Units (NPUs), headquartered in South Korea. They offer two types of NPUs: **DX-M1** and **DX-H1**, designed for efficient AI inference in edge devices, where the latter is the high-performance variant. 

<div align="center">
    <img src="../media/logo-deepx.png" alt="DX-M1 Module" width="70%"/>
</div>

For more information, please visit [DEEPX's official website](https://www.deepx.ai){:target="_blank"}.


## Requirements

The DX-M1 module supports:

### Supported Operating Systems 
 
- Ubuntu 20.04 or higher
- Debian 11 or higher

### Hardware requirements

- CPU: x86_64 (Intel, AMD) or ARM64 (aarch64)
- RAM: 8GB (16GB or higher recommended)
- Disk Space: 4GB or more
- Slot: M.2 Key-M (PCIe Gen3 x4 recommended)

## Driver installation

To install the DX-M1 module, insert it into an available M.2 (Key M) slot on the target system. The required power (up to 3.3V / 3A) is delivered through the M.2 interface, so no external power connection is needed.
After installing the module, install the following components from the DEEPX SDK, which are publicly available on GitHub:

- [DX Runtime (DX-RT)](https://github.com/DEEPX-AI/dx_rt){:target="_blank"}
- [NPU Driver](https://github.com/DEEPX-AI/dx_rt_npu_linux_driver){:target="_blank"}
- [Firmware (FW)](https://github.com/DEEPX-AI/dx_fw){:target="_blank"}

> Install the DX-RT version the runtime library was built against: v3.3.2 for OAAX 2.0.0, v3.0.0 for the 1.x builds (see the [download table](#download-links-and-compatibility-matrix)).

After installation, run the following command to verify proper setup:
```bash
dxrt-cli --status
```
Make sure the versions of the runtime, driver, and firmware are displayed correctly.
You should see output similar to the following:

```bash
DXRT v3.0.0
=======================================================
 * Device 0: M1, Accelerator type
---------------------   Version   ---------------------
 * RT Driver version   : v1.7.1
 * PCIe Driver version : v1.4.1
-------------------------------------------------------
 * FW version          : v2.1.4
--------------------- Device Info ---------------------
 * Memory : LPDDR5 6000 Mbps, 3.92GiB
 * Board  : M.2, Rev 1.5
 * Chip Offset : 0
 * PCIe   : Gen3 X4 [08:00:00]

NPU 0: voltage 750 mV, clock 1000 MHz, temperature 39'C
NPU 1: voltage 750 mV, clock 1000 MHz, temperature 39'C
NPU 2: voltage 750 mV, clock 1000 MHz, temperature 39'C
=======================================================
```

## Supported Models
The DEEPX SDK includes an [Open Model Zoo](https://developer.deepx.ai/modelzoo/){:target="_blank"} that provides various pre-optimized neural network models for DEEPX NPUs.


## Usage

### Runtime Library

The runtime implements the [standard OAAX interface](https://github.com/OAAX-standard/OAAX/blob/main/include/interface.h){:target="_blank"}. It does not use any initialization parameters: call `runtime_init` with a `Config` of `length = 0`.
It loads one model per process. Logs go to stderr and `./runtime.log`; set `DEEPX_RUNTIME_LOG_FILE` to change the file and `SPDLOG_LEVEL` for verbosity.



### Conversion Toolchain

The DEEPX conversion toolchain converts ONNX models to the DXNN format with the DEEPX compiler (DX-COM). The 2.0.0 image ships DX-COM 2.3.0, so nothing needs to be downloaded besides the image.

The toolchain takes a ZIP archive with, at its root, exactly one ONNX file, exactly one JSON configuration file, and optionally a calibration dataset folder for post-training quantization (recommended: without it the converted model may lose accuracy).
For instructions on preparing the archive, refer to this GitHub guide: [Prepare Your Model](https://github.com/OAAX-standard/deepx-acceleration/tree/main/conversion-toolchain#step-2-prepare-your-model){:target="_blank"}.
For guidance on JSON configuration files and supported ONNX operators, refer to the following
documents:
- [JSON File Configuration](https://github.com/OAAX-standard/deepx-acceleration/tree/main/conversion-toolchain#json-file-configuration){:target="_blank"}, or refer to the model zoo for examples.
- [Supported ONNX Operations](https://github.com/OAAX-standard/deepx-acceleration/tree/main/conversion-toolchain#supported-onnx-operations){:target="_blank"}

Once the model archive is prepared, load the image and run the conversion:

```bash
docker load -i oaax-deepx-toolchain.tar
docker run --rm -v ./model:/app/artifacts oaax-deepx-toolchain:2.0.0 /app/artifacts/model.zip /app/artifacts/output
```
This command assumes that the model archive is located at `./model/model.zip`. The converted model (`*.dxnn`) and a conversion log (`convert.log`) are saved in the `./model/output` directory.

## Download links and compatibility matrix

Runtime libraries are packaged as `library-dxrt-<DX-RT version>.tar.gz` and contain `libRuntimeLibrary.so`. Pick the build that matches your OS version and CPU architecture, and install the DX-RT version listed in its row on the host machine. Windows builds are not published yet.

### OAAX 2.0

| OAAX version | OS      | Version | CPU architecture | DX-RT | Runtime library | Conversion toolchain |
| ------------ | ------- | ------- | ---------------- | ----- | --------------- | -------------------- |
| 2.0.0        | Ubuntu  | 20.04   | x86_64           | 3.3.2 | [Download](https://oaax.nbg1.your-objectstorage.com/runtimes/2.0.0/DEEPX/x86_64/Ubuntu/20.04/library-dxrt-v3.3.2.tar.gz) | [Download](https://oaax.nbg1.your-objectstorage.com/conversion-toolchain/2.0.0/DEEPX/oaax-deepx-toolchain.tar) |
| 2.0.0        | Ubuntu  | 20.04   | aarch64          | 3.3.2 | [Download](https://oaax.nbg1.your-objectstorage.com/runtimes/2.0.0/DEEPX/aarch64/Ubuntu/20.04/library-dxrt-v3.3.2.tar.gz) | ⬆️ |
| 2.0.0        | Ubuntu  | 22.04   | x86_64           | 3.3.2 | [Download](https://oaax.nbg1.your-objectstorage.com/runtimes/2.0.0/DEEPX/x86_64/Ubuntu/22.04/library-dxrt-v3.3.2.tar.gz) | ⬆️ |
| 2.0.0        | Ubuntu  | 22.04   | aarch64          | 3.3.2 | [Download](https://oaax.nbg1.your-objectstorage.com/runtimes/2.0.0/DEEPX/aarch64/Ubuntu/22.04/library-dxrt-v3.3.2.tar.gz) | ⬆️ |
| 2.0.0        | Ubuntu  | 24.04   | x86_64           | 3.3.2 | [Download](https://oaax.nbg1.your-objectstorage.com/runtimes/2.0.0/DEEPX/x86_64/Ubuntu/24.04/library-dxrt-v3.3.2.tar.gz) | ⬆️ |
| 2.0.0        | Ubuntu  | 24.04   | aarch64          | 3.3.2 | [Download](https://oaax.nbg1.your-objectstorage.com/runtimes/2.0.0/DEEPX/aarch64/Ubuntu/24.04/library-dxrt-v3.3.2.tar.gz) | ⬆️ |

### OAAX 1.x (legacy)

> **Note:** these builds implement the 1.x runtime API, which is not compatible with OAAX 2.0 hosts. They need DX-RT v3.0.0, and their toolchain needs DX-COM 2.0.0 mounted into the container: `docker run -v ./model:/app/artifacts -v ./dx_com:/app/dx_com oaax-deepx-toolchain:1.1.0 /app/artifacts/model.zip /app/artifacts/output`.

| OAAX versions | OS            | Version  | CPU architecture | Runtime library                                                                                                           | Conversion toolchain                                                                                           |
| ------------- | ------------- | -------- | ---------------- | ------------------------------------------------------------------------------------------------------------------------- | -------------------------------------------------------------------------------------------------------------- |
| 1.1.0         | Ubuntu/Debian | 20.04/11 | x86_64           | [Download](https://oaax.nbg1.your-objectstorage.com/runtimes/1.1.0/DEEPX/x86_64/Ubuntu/20.04/library-dxrt-v3.0.0.tar.gz) | [Download](https://oaax.nbg1.your-objectstorage.com/conversion-toolchain/1.1.0/DEEPX/oaax-deepx-toolchain.tar) |
| 1.1.0         | Ubuntu/Debian | 20.04/11 | ARM64            | [Download](https://oaax.nbg1.your-objectstorage.com/runtimes/1.1.0/DEEPX/aarch64/Ubuntu/20.04/library-dxrt-v3.0.0.tar.gz) | ⬆️                                                                                                              |
| 1.1.0         | Ubuntu/Debian | 22.04/12 | x86_64           | [Download](https://oaax.nbg1.your-objectstorage.com/runtimes/1.1.0/DEEPX/x86_64/Ubuntu/22.04/library-dxrt-v3.0.0.tar.gz)  | ⬆️                                                                                                              |
| 1.1.0         | Ubuntu/Debian | 22.04/12 | ARM64            | [Download](https://oaax.nbg1.your-objectstorage.com/runtimes/1.1.0/DEEPX/aarch64/Ubuntu/22.04/library-dxrt-v3.0.0.tar.gz) | ⬆️                                                                                                              |
| 1.1.0         | Ubuntu/Debian | 24.04/13 | x86_64           | [Download](https://oaax.nbg1.your-objectstorage.com/runtimes/1.1.0/DEEPX/x86_64/Ubuntu/24.04/library-dxrt-v3.0.0.tar.gz)  | ⬆️                                                                                                              |
| 1.1.0         | Ubuntu/Debian | 24.04/13 | ARM64            | [Download](https://oaax.nbg1.your-objectstorage.com/runtimes/1.1.0/DEEPX/aarch64/Ubuntu/24.04/library-dxrt-v3.0.0.tar.gz) | ⬆️                                                                                                              |