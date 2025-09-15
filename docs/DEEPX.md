## Overview

DEEPX is a leading on-device AI semiconductor company specializing in Neural Processing Units (NPUs), headquartered in South Korea. They offer two types of NPUs: **DX-M1** and **DX-H1**, designed for efficient AI inference in edge devices, where the latter is the high-performance variant.   
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

> Make sure to install DXRT v2.9.5 version.

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
--------------------- Version ---------------------  
* RT Driver version : v1.7.1  
* PCIe Driver version : v1.4.1  
-------------------------------------------------------  
* FW version : v2.1.0  
--------------------- Device Info ---------------------  
* Memory : LPDDR5 6000 MHz, 3.92GiB  
* Board : M.2, Rev 1.5  
* Chip Offset : 0  
* PCIe : Gen3 X4 [08:00:00]  

NPU 0: voltage 750 mV, clock 1000 MHz, temperature 38'C  
NPU 1: voltage 750 mV, clock 1000 MHz, temperature 38'C  
NPU 2: voltage 750 mV, clock 1000 MHz, temperature 38'C  
dvfs Disabled  
=======================================================
```

## Supported Models
The DEEPX SDK includes an [Open Model Zoo](https://developer.deepx.ai/wp-content/modelzoo/model_zoo_fin.html){:target="_blank"} that provides various pre-optimized neural network models for DEEPX NPUs.


## Usage

### Runtime Library

The runtime implements the [standard OAAX interface](https://github.com/OAAX-standard/OAAX/blob/main/Illustrative%20example/Runtime/include/interface.h){:target="_blank"}, without any initialization arguments. Therefore the `int runtime_initialization_with_args` function is not supported, and the `int runtime_initialization()` function should be used instead.



### Conversion Toolchain

The DEEPX conversion toolchain doesn't come with DX COM by default, it needs to be manually downloaded and unpacked on the machine where the conversion will be performed.  
You can find details about the process in the [DEEPX conversion toolchain GitHub repository](https://github.com/OAAX-standard/deepx-acceleration/tree/main/conversion-toolchain#step-1-download-deepxs-dx-com){:target="_blank"}.

To convert models for use with DEEPX Accelerators, models must be packaged as a ZIP archive.  
For instructions on preparing the archive, refer to this GitHub guide: [Prepare Your Model](https://github.com/OAAX-standard/deepx-acceleration/tree/main/conversion-toolchain#step-2-prepare-your-model){:target="_blank"}.
For guidance on JSON configuration files and supported ONNX operators, refer to the following
documents:
- [JSON File Configuration](https://github.com/OAAX-standard/deepx-acceleration/tree/main/conversion-toolchain#json-file-configuration){:target="_blank"}, or refer to the model zoo for examples.
- [Supported ONNX Operations](https://github.com/OAAX-standard/deepx-acceleration/tree/main/conversion-toolchain#supported-onnx-operations){:target="_blank"}

Once DX COM is downloaded and the model archive is prepared, you can run the conversion using the following command:

```bash
docker run -v ./model:/app/artifacts -v ./dx_com:/app/dx_com oaax-deepx-toolchain:latest /app/artifacts/model.zip /app/artifacts/output
```
This command assumes that the model archive is located at `./model/model.zip` and the DX COM directory is located at `./dx_com`. The converted model will be saved in the `./model/output` directory.

## Download links and compatibility matrix

> Please make sure that you're using DXRT v2.9.5 version on the host-running machine.

| OAAX versions | OS            | Version  | CPU architecture | Runtime library                                                                                                            | Conversion toolchain                                                                                            |
| ------------- | ------------- | -------- | ---------------- | -------------------------------------------------------------------------------------------------------------------------- | --------------------------------------------------------------------------------------------------------------- |
| 1.0.0         | Ubuntu/Debian | 20.04/11 | x86_64           | [Download](https://oaax.nbg1.your-objectstorage.com/runtimes/latest/DEEPX/aarch64/Ubuntu/20.04/library-dxrt-v2.9.5.tar.gz) | [Download](https://oaax.nbg1.your-objectstorage.com/conversion-toolchain/latest/DEEPX/oaax-deepx-toolchain.tar) |
| 1.0.0         | Ubuntu/Debian | 20.04/11 | ARM64            | [Download](https://oaax.nbg1.your-objectstorage.com/runtimes/latest/DEEPX/aarch64/Ubuntu/20.04/library-dxrt-v2.9.5.tar.gz) | ⬆️                                                                                                               |
| 1.0.0         | Ubuntu/Debian | 22.04/12 | x86_64           | [Download](https://oaax.nbg1.your-objectstorage.com/runtimes/latest/DEEPX/x86_64/Ubuntu/22.04/library-dxrt-v2.9.5.tar.gz)  | ⬆️                                                                                                               |
| 1.0.0         | Ubuntu/Debian | 22.04/12 | ARM64            | [Download](https://oaax.nbg1.your-objectstorage.com/runtimes/latest/DEEPX/aarch64/Ubuntu/22.04/library-dxrt-v2.9.5.tar.gz) | ⬆️                                                                                                               |
| 1.0.0         | Ubuntu/Debian | 24.04/13 | x86_64           | [Download](https://oaax.nbg1.your-objectstorage.com/runtimes/latest/DEEPX/x86_64/Ubuntu/24.04/library-dxrt-v2.9.5.tar.gz)  | ⬆️                                                                                                               |
| 1.0.0         | Ubuntu/Debian | 24.04/13 | ARM64            | [Download](https://oaax.nbg1.your-objectstorage.com/runtimes/latest/DEEPX/aarch64/Ubuntu/24.04/library-dxrt-v2.9.5.tar.gz) | ⬆️                                                                                                               |