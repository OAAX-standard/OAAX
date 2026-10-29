> **📦 Implementation details**  
> You can find the implementation of the NVIDIA GPUs runtime & toolchain in the [Github repository](https://github.com/OAAX-standard/nvidia-acceleration){:target="_blank"}.

## Overview

For NVIDIA GPUs, OAAX 2.0 provides two backends. Both expose the same runtime interface, so your host application does not change:

| Backend | How it works | Choose it when |
| ------- | ------------ | -------------- |
| **ONNX Runtime** (`NVIDIA_ORT`) | The toolchain simplifies the ONNX model; the runtime runs it with ONNX Runtime and the [CUDA Execution Provider](https://onnxruntime.ai/docs/execution-providers/CUDA-ExecutionProvider.html){:target="_blank"}. | You want the broadest model compatibility, Windows support, or to convert on a machine without a GPU. **Start here.** |
| **TensorRT** (`NVIDIA_TRT`) | The toolchain compiles the model into a TensorRT engine (`model.trt`) for the GPU it runs on; the runtime loads the engine. | You want maximum inference performance. Conversion needs the target GPU, and the model must run on that same GPU model afterwards. |

## Requirements

- An NVIDIA GPU with a recent driver, on Ubuntu (x86_64 or aarch64) or Windows 10/11 (ONNX Runtime backend only). NVIDIA Jetson (JetPack 5, 6 or 7) and aarch64 SBSA servers (DGX Spark, Grace) are supported.
- The CUDA libraries of the major version your runtime build was made for (see the tables below), installed on the host. Follow NVIDIA's instructions for [CUDA](https://developer.nvidia.com/cuda-downloads){:target="_blank"} or [Jetson](https://developer.nvidia.com/embedded/jetpack){:target="_blank"}.
- Docker, to run the conversion toolchain. The TensorRT toolchain also needs the [NVIDIA Container Toolkit](https://docs.nvidia.com/datacenter/cloud-native/container-toolkit/latest/install-guide.html){:target="_blank"} (`docker run --gpus all`).

### Pick the right runtime build

Runtime builds are made per CUDA major version (11, 12 or 13) and, for TensorRT, per TensorRT major version (10 or 11). Pick the build that matches what is installed on the host:

- `nvidia-smi` shows the highest CUDA version your driver supports; the CUDA libraries you install (for example `libcublasLt.so.12`) decide which build loads. Check with `ldconfig -p | grep libcublasLt`.
- A mismatch shows up when the runtime initializes, for example `Failed to load library libonnxruntime_providers_cuda.so ... libcublasLt.so.12: cannot open shared object file`. Install that CUDA version, or download the build for the CUDA version you have.

## Usage

### Runtime Library

The runtime library implements the OAAX's runtime interface for initializing, loading models, running inference and cleaning up the runtime.
It is initialized by calling `RuntimeStatus runtime_init(Config config);`, with the following parameters passed as string key/value pairs in `Config` (ONNX Runtime backend; see the [repository](https://github.com/OAAX-standard/nvidia-acceleration){:target="_blank"} for the TensorRT backend):

- `log_level` (string, default is "2"): The minimal log level for the runtime. This can be set to `0` for trace, `1` for debug, `2` for info, `3` for warnings, `4` for errors, `5` for critical and `6` to disable logging.
- `log_file` (string, default is "runtime.log"): The file to which the runtime logs will be written.
- `num_threads` (string, default is "4"): The maximum number of threads that can be used by the runtime. The higher the number, the more CPU resources will be used, but the better the throughput.

> You can check out the examples repository for more details on how to use the runtime library: [OAAX Examples](https://github.com/OAAX-standard/examples).

### Conversion Toolchain: ONNX Runtime

The toolchain validates, optimizes and simplifies the ONNX model. It does not need a GPU.

```bash
docker load -i oaax-nvidia-onnxruntime-toolchain.tar
docker run --rm --user $(id -u):$(id -g) -v ./model:/model oaax-nvidia-toolchain:2.0.0 /model/model.onnx /model/output
```

The command assumes the model is at `./model/model.onnx`. The simplified model (`model-simplified.onnx`) and a conversion log (`logs.json`) are written to `./model/output`. Pass that file to `runtime_load_models`.

### Conversion Toolchain: TensorRT

The toolchain compiles the model with `trtexec` for the GPU that is installed in the machine, so run it on the deployment machine. Pack the model with a `config.json` into a zip bundle:

```bash
echo '{"precision": "fp16", "workspace_gb": 4}' > config.json
zip bundle.zip model.onnx config.json

docker load -i oaax-nvidia-tensorrt-toolchain.tar
docker run --rm --gpus all -v $(pwd)/bundle.zip:/input/bundle.zip -v $(pwd)/output:/output \
  oaax-nvidia-tensorrt-toolchain:2.0.0 /input/bundle.zip /output
```

The engine (`model.trt`) and a conversion log (`logs.json`) are written to `./output`. Pass `model.trt` to `runtime_load_models` of a TensorRT runtime build. The TensorRT version used for conversion must match the one bundled in the runtime; the Docker image guarantees this.

## Download links and compatibility matrix

Runtime libraries are packaged as `library-*.tar.gz` and contain `libRuntimeLibrary.so` (`RuntimeLibrary.dll` on Windows) with the libraries it needs. Sizes are in brackets.

### OAAX 2.0: ONNX Runtime backend

Conversion toolchain (any platform): [Download](https://nbg1.your-objectstorage.com/oaax/conversion-toolchain/2.0.0/NVIDIA/oaax-nvidia-onnxruntime-toolchain.tar) (967 MB)

| Platform | CUDA | Runtime library |
| -------- | ---- | --------------- |
| Ubuntu, x86_64 | 11 | [Download](https://nbg1.your-objectstorage.com/oaax/runtimes/2.0.0/NVIDIA_ORT/x86_64/Ubuntu/library-cuda_11.tar.gz) (974 MB) |
| Ubuntu, x86_64 | 12 | [Download](https://nbg1.your-objectstorage.com/oaax/runtimes/2.0.0/NVIDIA_ORT/x86_64/Ubuntu/library-cuda_12.tar.gz) (1.1 GB) |
| Ubuntu, x86_64 | 13 | [Download](https://nbg1.your-objectstorage.com/oaax/runtimes/2.0.0/NVIDIA_ORT/x86_64/Ubuntu/library-cuda_13.tar.gz) (823 MB) |
| Jetson, JetPack 5 | 11 | [Download](https://nbg1.your-objectstorage.com/oaax/runtimes/2.0.0/NVIDIA_ORT/aarch64-jetson/Ubuntu/library-cuda_11.tar.gz) (767 MB) |
| Jetson, JetPack 6 | 12 | [Download](https://nbg1.your-objectstorage.com/oaax/runtimes/2.0.0/NVIDIA_ORT/aarch64-jetson/Ubuntu/library-cuda_12.tar.gz) (942 MB) |
| Jetson, JetPack 7 | 13 | [Download](https://nbg1.your-objectstorage.com/oaax/runtimes/2.0.0/NVIDIA_ORT/aarch64-jetson/Ubuntu/library-cuda_13.tar.gz) (125 MB) |
| Ubuntu, aarch64 SBSA (DGX Spark, Grace) | 13 | [Download](https://nbg1.your-objectstorage.com/oaax/runtimes/2.0.0/NVIDIA_ORT/aarch64-sbsa/Ubuntu/library-cuda_13.tar.gz) (729 MB) |
| Windows 10/11, x86_64 | 11 | [Download](https://nbg1.your-objectstorage.com/oaax/runtimes/2.0.0/NVIDIA_ORT/x86_64/Windows/library-cuda_11.tar.gz) (831 MB) |
| Windows 10/11, x86_64 | 12 | [Download](https://nbg1.your-objectstorage.com/oaax/runtimes/2.0.0/NVIDIA_ORT/x86_64/Windows/library-cuda_12.tar.gz) (1.7 GB) |
| Windows 10/11, x86_64 | 13 | [Download](https://nbg1.your-objectstorage.com/oaax/runtimes/2.0.0/NVIDIA_ORT/x86_64/Windows/library-cuda_13.tar.gz) (615 MB) |

### OAAX 2.0: TensorRT backend

Conversion toolchain (needs the target GPU): [Download](https://nbg1.your-objectstorage.com/oaax/conversion-toolchain/2.0.0/NVIDIA/oaax-nvidia-tensorrt-toolchain.tar) (11.0 GB)

| Platform | OS | TensorRT | CUDA | Runtime library |
| -------- | -- | -------- | ---- | --------------- |
| x86_64 | Ubuntu 22.04 | 10 | 12 | [Download](https://nbg1.your-objectstorage.com/oaax/runtimes/2.0.0/NVIDIA_TRT/x86_64/Ubuntu/22.04/library-trt10-cuda_12.tar.gz) (315 MB) |
| x86_64 | Ubuntu 22.04 | 10 | 13 | [Download](https://nbg1.your-objectstorage.com/oaax/runtimes/2.0.0/NVIDIA_TRT/x86_64/Ubuntu/22.04/library-trt10-cuda_13.tar.gz) (326 MB) |
| x86_64 | Ubuntu 22.04 | 11 | 12 | [Download](https://nbg1.your-objectstorage.com/oaax/runtimes/2.0.0/NVIDIA_TRT/x86_64/Ubuntu/22.04/library-trt11-cuda_12.tar.gz) (342 MB) |
| x86_64 | Ubuntu 22.04 | 11 | 13 | [Download](https://nbg1.your-objectstorage.com/oaax/runtimes/2.0.0/NVIDIA_TRT/x86_64/Ubuntu/22.04/library-trt11-cuda_13.tar.gz) (329 MB) |
| Jetson, JetPack 6 | Ubuntu 22.04 | 10 | 12 | [Download](https://nbg1.your-objectstorage.com/oaax/runtimes/2.0.0/NVIDIA_TRT/aarch64-jetson/Ubuntu/22.04/library-trt10-cuda_12.tar.gz) (1 MB) |
| aarch64 SBSA (DGX Spark, Grace) | Ubuntu 24.04 | 10 | 12 | [Download](https://nbg1.your-objectstorage.com/oaax/runtimes/2.0.0/NVIDIA_TRT/aarch64-sbsa/Ubuntu/24.04/library-trt10-cuda_12.tar.gz) (106 MB) |
| aarch64 SBSA (DGX Spark, Grace) | Ubuntu 24.04 | 10 | 13 | [Download](https://nbg1.your-objectstorage.com/oaax/runtimes/2.0.0/NVIDIA_TRT/aarch64-sbsa/Ubuntu/24.04/library-trt10-cuda_13.tar.gz) (539 MB) |
| aarch64 SBSA (DGX Spark, Grace) | Ubuntu 24.04 | 11 | 13 | [Download](https://nbg1.your-objectstorage.com/oaax/runtimes/2.0.0/NVIDIA_TRT/aarch64-sbsa/Ubuntu/24.04/library-trt11-cuda_13.tar.gz) (547 MB) |

The Jetson TensorRT build only bundles `libcudart`: JetPack provides TensorRT and the other CUDA libraries.

### OAAX 1.x (legacy)

> **Note:** these builds implement the 1.x runtime API, which is not compatible with OAAX 2.0 hosts. Their toolchain is `oaax-nvidia-toolchain:1.1.1`, run with `docker run -v ./model:/model oaax-nvidia-toolchain:1.1.1 /model/model.onnx /model/output`.

| OAAX versions | OS             | Version         | CPU architecture | Runtime library                                                                                                                                                                                                                      | Conversion toolchain                                                                                             |
| ------------- | -------------- | --------------- | ---------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ | ---------------------------------------------------------------------------------------------------------------- |
| 1.1.1         | Ubuntu         | 20.04 or higher | x86_64           | [CUDA 11](https://oaax.nbg1.your-objectstorage.com/runtimes/1.1.1/NVIDIA/x86_64/Ubuntu/library-cuda_11.tar.gz), [CUDA 12](https://oaax.nbg1.your-objectstorage.com/runtimes/1.1.1/NVIDIA/x86_64/Ubuntu/library-cuda_12.tar.gz)       | [Download](https://oaax.nbg1.your-objectstorage.com/conversion-toolchain/1.1.1/NVIDIA/oaax-nvidia-toolchain.tar) |
| 1.1.1         | NVIDIA JetPack | -               | ARM64            | [JetPack 5](https://oaax.nbg1.your-objectstorage.com/runtimes/1.1.1/NVIDIA/aarch64/Ubuntu/library-cuda_11.tar.gz), [JetPack 6](https://oaax.nbg1.your-objectstorage.com/runtimes/1.1.1/NVIDIA/aarch64/Ubuntu/library-cuda_12.tar.gz) | ⬆️                                                                                                                |
| 1.1.1         | Windows        | 10 or higher    | x86_64           | [CUDA 11](https://oaax.nbg1.your-objectstorage.com/runtimes/1.1.1/NVIDIA/x86_64/Windows/library-cuda_11.tar.gz), [CUDA 12](https://oaax.nbg1.your-objectstorage.com/runtimes/1.1.1/NVIDIA/x86_64/Windows/library-cuda_12.tar.gz)     | ⬆️                                                                                                                |
