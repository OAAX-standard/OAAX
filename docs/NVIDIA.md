> **📦 Implementation details**  
> The runtime and toolchain are implemented in the [nvidia-acceleration repository](https://github.com/OAAX-standard/nvidia-acceleration){:target="_blank"}. Latest release: [v2.0.0](https://github.com/OAAX-standard/nvidia-acceleration/releases/tag/v2.0.0){:target="_blank"}.

## At a glance

| | |
| --- | --- |
| **Devices** | NVIDIA GPUs (TensorRT toolchain: compute capability 7.5 or higher), Jetson, DGX Spark / Grace |
| **Backends** | ONNX Runtime with the CUDA Execution Provider (`nvidia-ort`), native TensorRT 10 or 11 (`nvidia-trt`) |
| **Runtime library** | `libRuntimeLibrary.so` (Linux), `RuntimeLibrary.dll` (Windows, ONNX Runtime only), 1 MB to 1.7 GB depending on the build |
| **Toolchain image** | `oaax-nvidia-toolchain:2.0.0` (967 MB) or `oaax-nvidia-tensorrt-toolchain:2.0.0` (11 GB) |
| **Verified on hardware** | RTX A4000, driver 580, CUDA 13, Ubuntu x86_64: both backends, TensorRT 10 |
| **Not verified** | Windows, Jetson, DGX Spark / Grace (SBSA), TensorRT 11 conversion, running the CUDA 11 package, the wheel conversion path, `docker run --gpus all` as written |

## Overview

NVIDIA GPUs are supported through two backends that expose the same OAAX 2.0 runtime interface, so the host application does not change. Use the runtime and the toolchain of the **same** backend together.

| | ONNX Runtime backend | TensorRT backend |
| --- | --- | --- |
| `oaax-manager` vendor | `nvidia-ort` | `nvidia-trt` |
| GPU class | NVIDIA GPUs, Jetson, DGX Spark / Grace | same, except Windows |
| Platforms | Linux x86_64, Jetson, SBSA, **Windows x86_64** | Linux x86_64, Jetson, SBSA |
| Toolchain input | `.onnx` file | `.zip` bundle: `model.onnx` + `config.json` |
| Toolchain output | `model-simplified.onnx` | `model.trt` engine, built for the GPU it ran on |
| Toolchain image | `oaax-nvidia-toolchain:2.0.0` (967 MB, no GPU needed) | `oaax-nvidia-tensorrt-toolchain:2.0.0` (11 GB, needs the target GPU) |
| Runtime library | `libRuntimeLibrary.so` / `RuntimeLibrary.dll` + bundled ONNX Runtime and cuDNN | `libRuntimeLibrary.so` + bundled TensorRT and `libcudart` |
| Runtime accepts | `.onnx` path or in-memory buffer | `.trt` path or in-memory buffer |

**Which one?**

- **ONNX Runtime** (start here): the toolchain only simplifies the graph, so you can convert on any machine, and the runtime executes the model with the [CUDA Execution Provider](https://onnxruntime.ai/docs/execution-providers/CUDA-ExecutionProvider.html){:target="_blank"}. Broadest model coverage and the only backend on Windows.
- **TensorRT**: the toolchain compiles an engine for the exact GPU in the machine (`trtexec`), and the runtime loads it directly. Best inference performance, but the engine only runs on the GPU architecture, CPU platform and TensorRT version it was built for.

## Requirements

### Hardware

| Item | Requirement |
| --- | --- |
| GPU | An NVIDIA GPU. The TensorRT toolchain refuses GPUs below compute capability 7.5 (Turing). Verified on an RTX A4000 (8.6). |
| GPU memory | Depends on the model. ORT: the CUDA memory arena grows on demand (cap it with `gpu_mem_limit`). TRT: the engine plus one set of input/output buffers per concurrent slot (`max_concurrent`). |
| Jetson | ORT: JetPack 5 (CUDA 11), 6 (CUDA 12) or 7 (CUDA 13). TRT: JetPack 6 only (CUDA 12, TensorRT 10). |
| DGX Spark / Grace (`sbsa`) | ORT: CUDA 13. TRT: Ubuntu 24.04, TensorRT 10 (CUDA 12 or 13) or TensorRT 11 (CUDA 13). |
| Several GPUs | Both runtimes always use GPU 0. Pick the GPU with `CUDA_VISIBLE_DEVICES`. |

### Software

| | Needed to **run** (runtime) | Needed to **convert** (toolchain) |
| --- | --- | --- |
| OS | Linux: Ubuntu 20.04+ for CUDA 11/12 ORT builds; Ubuntu 22.04+ for the CUDA 13 ORT builds and all TRT builds (glibc 2.29 / 2.34 / 2.33). Windows 10/11 (ORT only). | Docker on any Linux x86_64 host (ORT). Docker plus the NVIDIA Container Toolkit, or the Python wheel with a local TensorRT (TRT). |
| NVIDIA driver | New enough for the package's CUDA major version, per [NVIDIA's compatibility table](https://docs.nvidia.com/deploy/cuda-compatibility/){:target="_blank"}: CUDA 11 needs 450.80+, CUDA 12 needs 525.60+, CUDA 13 needs 580+. | TRT only: a driver for CUDA 13 (580+), with `nvidia-smi` working on the host. |
| CUDA libraries | **ORT: not bundled.** Install the CUDA major of your package: `libcudart`, `libcublas`, `libcublasLt` and `libcufft` (Windows: on `PATH`). **TRT: bundled** (`libcudart` and TensorRT), nothing to install. | None on the host (they are inside the image). |
| cuDNN | ORT: bundled, except on Jetson (JetPack provides it) and for the `cuda12` x86_64 package (see the warning below). TRT: not used. | None |
| TensorRT | TRT: bundled (10.16 or 11.0). **Jetson: not bundled**, JetPack provides it. | The image contains TensorRT 10.16. The TRT 11 runtime needs the wheel with TensorRT 11 installed. |

!!! warning "The TensorRT version must match"
    A `.trt` engine only loads in a runtime with the same TensorRT version as the toolchain that built it. The Docker toolchain is TensorRT 10, so use a `trt10` runtime with it.

!!! warning "Known issue: the `cuda12` ONNX Runtime package (Linux x86_64) needs cuDNN 8"
    The 2.0.0 `cuda12` package contains cuDNN 9 files, but its ONNX Runtime (1.16.3) loads **cuDNN 8** (`libcudnn.so.8`), so it fails with `libcudnn.so.8: cannot open shared object file`. Download cuDNN **8.9.7.29 for CUDA 12** from NVIDIA and place it **next to `libRuntimeLibrary.so`** (about 800 MB of libraries):

    ```bash
    curl -fLO https://developer.download.nvidia.com/compute/cudnn/redist/cudnn/linux-x86_64/cudnn-linux-x86_64-8.9.7.29_cuda12-archive.tar.xz
    tar xJf cudnn-linux-x86_64-8.9.7.29_cuda12-archive.tar.xz --wildcards "*/lib/libcudnn.so.8*" "*/lib/libcudnn_*_infer.so.8*"
    cp -P cudnn-linux-x86_64-8.9.7.29_cuda12-archive/lib/libcudnn.so.8* cudnn-linux-x86_64-8.9.7.29_cuda12-archive/lib/libcudnn_*_infer.so.8* runtime-nvidia-ort-2.0.0-linux-x86_64-cuda12/
    ```

    (sha256 of the archive, 865 MB: `475333625c7e42a7af3ca0b2f7506a106e30c93b1aa0081cd9c13efb6e21e3bb`.) No `LD_LIBRARY_PATH` is needed for cuDNN, because the runtime libraries look in their own folder. The `cuda11` and `cuda13` packages are not affected: they bundle the cuDNN version they load (8.9.7 and 9.25). The CUDA 12 libraries (`libcudart`, `libcublas`, `libcufft`) must still be installed on the host.

### Which package do I pick?

| Platform | ONNX Runtime `--variant` | TensorRT `--variant` |
| --- | --- | --- |
| Linux x86_64 | `cuda11`, `cuda12`, `cuda13` | `ubuntu22.04-trt10-cuda12`, `ubuntu22.04-trt10-cuda13`, `ubuntu22.04-trt11-cuda12`, `ubuntu22.04-trt11-cuda13` |
| Jetson | `jetson-cuda11` (JetPack 5), `jetson-cuda12` (6), `jetson-cuda13` (7) | `jetson-ubuntu22.04-trt10-cuda12` (JetPack 6) |
| DGX Spark / Grace | `sbsa-cuda13` | `sbsa-ubuntu24.04-trt10-cuda12`, `sbsa-ubuntu24.04-trt10-cuda13`, `sbsa-ubuntu24.04-trt11-cuda13` |
| Windows x86_64 | `cuda11`, `cuda12`, `cuda13` with `--os windows` | not available |

Rule of thumb: ORT, take the CUDA major you have installed (`ldconfig -p | grep libcublasLt`). TRT, take the TensorRT major of your toolchain and the CUDA major supported by your driver.

## Get the runtime and toolchain

[`oaax-manager`](https://pypi.org/project/oaax-manager/) downloads, verifies (sha256) and unpacks the right build for the machine it runs on. All commands are run with [uv](https://docs.astral.sh/uv/getting-started/installation/):

=== "ONNX Runtime"

    ```bash
    uvx oaax-manager fetch runtime nvidia-ort --variant cuda13   # -> runtime-nvidia-ort-2.0.0-linux-x86_64-cuda13/
    uvx oaax-manager fetch toolchain nvidia-ort                  # -> oaax-nvidia-onnxruntime-toolchain-2.0.0.tar
    docker load -i oaax-nvidia-onnxruntime-toolchain-2.0.0.tar   # loads oaax-nvidia-toolchain:2.0.0
    ```

=== "TensorRT"

    ```bash
    uvx oaax-manager fetch runtime nvidia-trt --variant ubuntu22.04-trt10-cuda13
    uvx oaax-manager fetch toolchain nvidia-trt                  # 11 GB
    docker load -i oaax-nvidia-tensorrt-toolchain-2.0.0.tar      # loads oaax-nvidia-tensorrt-toolchain:2.0.0
    ```

`uvx oaax-manager list` shows every variant (`*` marks the one for your machine). Use `--os windows` or `--arch aarch64` to fetch for another platform. Sizes range from 1 MB (Jetson TensorRT) to 11 GB (TensorRT toolchain).

### Download links and compatibility matrix

Runtime libraries are packaged as `library-*.tar.gz` and contain `libRuntimeLibrary.so` (`RuntimeLibrary.dll` on Windows) with the libraries it needs. Sizes are in brackets. The sha256 of every file is in the [release notes](https://github.com/OAAX-standard/nvidia-acceleration/releases/tag/v2.0.0){:target="_blank"}.

#### OAAX 2.0: ONNX Runtime backend

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

#### OAAX 2.0: TensorRT backend

Conversion toolchain (needs the target GPU): [Docker image](https://nbg1.your-objectstorage.com/oaax/conversion-toolchain/2.0.0/NVIDIA/oaax-nvidia-tensorrt-toolchain.tar) (11.0 GB) or [Python wheel](https://nbg1.your-objectstorage.com/oaax/conversion-toolchain/2.0.0/NVIDIA/oaax_nvidia_tensorrt_conversion-2.0.0-py3-none-any.whl) (13 KB, needs a local TensorRT)

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

## Conversion toolchain

Both toolchains are Docker images started with two positional arguments, `<input> <output-directory>`. Mount a directory your user can write and pass `--user` so the output files belong to you.

=== "ONNX Runtime"

    No GPU needed. The toolchain simplifies the graph with [onnxsim](https://github.com/daquexian/onnx-simplifier){:target="_blank"}.

    ```bash
    docker run --rm --user $(id -u):$(id -g) -v ./model:/model oaax-nvidia-toolchain:2.0.0 /model/model.onnx /model/output
    ```

    | Argument | Meaning |
    | --- | --- |
    | `<onnx-path>` | Input model, inside the container. |
    | `<output-directory>` | Created if missing. Receives `model-simplified.onnx` (named `<input name>-simplified.onnx`) and `logs.json`. |

    There is **no configuration file and no option**. If simplification fails the original model is passed through unchanged and the failure is logged (`Failed to simplify ONNX model`, with `Error` and `Fallback`).

    !!! warning "Mount the directory, not the file"
        The simplified model is first written next to the input, then copied to the output directory. The input directory must therefore be writable: with `--user` and a single-file mount (`-v ./model.onnx:/input/model.onnx`) it fails with `PermissionError: ... '/input/model-simplified.onnx'`. The copy left next to your input can be deleted.

    `logs.json` is a list of `{"Message": ..., "Data": {...}}` entries: `Simplifying ONNX model` (`ONNX Path`, `MD5`), then `Simplified ONNX model successfully` or the failure above. The final `Successful Conversion` entry (`Output file name`, `Output file MD5`, `MIME type` = `application/x-onnx; device=cpu`) is only printed to the console.

=== "TensorRT"

    Needs the GPU the model will run on (the engine is tuned for it). Pack the model and a `config.json` into a zip:

    ```bash
    mkdir -p trt-input trt-output && cp model.onnx trt-input/
    echo '{"precision": "fp16", "workspace_gb": 4}' > trt-input/config.json
    (cd trt-input && zip bundle.zip model.onnx config.json)

    docker run --rm --gpus all --user $(id -u):$(id -g) -v ./trt-input:/input -v ./trt-output:/output \
      oaax-nvidia-tensorrt-toolchain:2.0.0 /input/bundle.zip /output
    ```

    **`config.json`**

    | Key | Required | Values | Default | Effect |
    | --- | --- | --- | --- | --- |
    | `precision` | yes | `fp32`, `fp16`, `int8` | | Engine precision. |
    | `workspace_gb` | yes | positive number | | Builder workspace limit (`trtexec --memPoolSize=workspace:<n>g`). |
    | `optimization_level` | no | integer `1`-`5` | `3` (TensorRT) | Builder effort (`--builderOptimizationLevel`): higher is slower to build, can be faster to run. |
    | `avg_timing` | no | integer >= 1 | `8` (TensorRT) | Timing iterations per kernel choice (`--avgTiming`). |
    | `calibration_data` | int8 only | folder in the zip, e.g. `"calib/"` | | Representative images (jpg, jpeg, png, bmp, webp); 100-500 are recommended. |
    | `preprocessing` | no | `{"mean": [r,g,b], "std": [r,g,b]}` | ImageNet mean/std | int8 only. Images are resized (bilinear) to the model's input size, scaled to 0-1, then normalized. The first model input must be 4-D `NCHW`. |

    **Outputs** in `/output`: `model.trt` and `logs.json` (plus `model.trt.calib`, the calibration cache, for int8 on TensorRT 10). `logs.json` holds the same `Message`/`Data` entries as above: the detected GPU (`sm_86`), the input MD5 and config, the exact `trtexec` command with the tail of its output, and `Successful Conversion` with the output MD5 and a MIME type such as `application/x-tensorrt; arch=x86_64; sm=86; trt=10` (CPU architecture, GPU compute capability, TensorRT major).

    | Precision | How the engine is built |
    | --- | --- |
    | `fp32` | `trtexec` |
    | `fp16` | TensorRT 10: `trtexec --fp16`. TensorRT 11 (wheel): the ONNX model is first converted to float16 with `onnxconverter-common`, keeping float32 inputs and outputs. |
    | `int8` | TensorRT 10: entropy calibration with your images (needs the `tensorrt` and `pycuda` Python packages). TensorRT 11: Q/DQ nodes inserted by `nvidia-modelopt`. |

    `optimization_level` and `avg_timing` only apply to builds through `trtexec` (fp32, fp16, and int8 on TensorRT 11). The int8 build of TensorRT 10 ignores them.

    !!! warning "Fixed output sizes only"
        The TensorRT runtime allocates its buffers from the engine's tensor shapes. A model whose output size depends on the data, typically one with a `NonMaxSuppression` operator inside the graph (like the Hello World example model), converts without error but the engine has a dynamic output dimension (`-1`) and fails to load: `runtime_load_models failed: cudaMalloc failed for tensor: <name>`. Use the ONNX Runtime backend for such models, or move the NMS out of the model.

    **Without Docker** (for example where containers cannot see the GPU): install the TensorRT SDK so `trtexec` is on `PATH`, then run the [wheel](https://nbg1.your-objectstorage.com/oaax/conversion-toolchain/2.0.0/NVIDIA/oaax_nvidia_tensorrt_conversion-2.0.0-py3-none-any.whl). It checks its prerequisites (driver, `trtexec`, and for int8 the packages above), reports what is missing and warns when the TensorRT major is not 10 or 11. For int8, install the matching extra: `[int8-trt10]` or `[int8-trt11]`.

    ```bash
    uvx --from ./oaax_nvidia_tensorrt_conversion-2.0.0-py3-none-any.whl conversion_toolchain --input-path bundle.zip --output-dir ./trt-output
    ```

    **Errors** are raised as Python exceptions (non-zero exit status) with these messages:

    | Message | Cause |
    | --- | --- |
    | `Missing required config key: 'precision'` | `precision` or `workspace_gb` absent. |
    | `precision must be one of ['fp32', 'fp16', 'int8']` / `workspace_gb must be a positive number` / `optimization_level must be an integer 1-5` / `avg_timing must be a positive integer` | Invalid value. |
    | `'calibration_data' is required in config.json when precision is 'int8'` | int8 without calibration images. |
    | `model.onnx not found in zip archive` / `config.json not found in zip archive` | The zip must have both files at its top level. |
    | `Dependency issues for <precision> conversion: ...` | `nvidia-smi`, `trtexec` or an int8 package is missing. |
    | `Detected GPU compute capability ... is not supported by TensorRT 10 (minimum: sm_75 / Turing)` | GPU too old. |
    | `trtexec failed (exit N). stderr: ...` | TensorRT rejected the model (unsupported operator, not enough workspace...). The full output is in `logs.json`. |

## Runtime library

The runtime implements the [OAAX 2.0 runtime interface](Specification.md#oaax-runtime-specification). Load `libRuntimeLibrary.so` from the folder you extracted it to (it finds its bundled libraries next to itself), call `runtime_init`, then `runtime_load_models` with the converted model.

### `runtime_init` config keys

All keys and values are strings. Unknown keys are ignored with a warning in the log.

| Key | Backend | Values | Default | Effect |
| --- | --- | --- | --- | --- |
| `log_level` | both | `0` trace, `1` debug, `2` info, `3` warning, `4` error, `5` critical, `6` off | `2` | Minimum level, for the log file and the console. An out-of-range number falls back to `2`; a non-number is an error (TRT: aborts the host). |
| `log_file` | both | path | `runtime.log` | Log file, rotated at 5 MB, 3 files kept. On Windows the default is a per-process file in the temp directory. If it cannot be created, ORT logs to the console only; **TRT terminates the host process**, so give it a writable path. |
| `log_stdout` | both | `true` | off | Also log to the console. Only the exact value `true` enables it. |
| `perf_mode` | both | `power`, `eco` | unset | Picks defaults from the hardware; explicit keys below override it. ORT `power`: threads = CPU cores, no GPU memory cap. ORT `eco`: half the cores, half the GPU memory. TRT `power`: `max_concurrent` = clamp(SM count / 8, 2, 16). TRT `eco`: half of that. Any other value is ignored with a warning. |
| `num_threads` | ORT | `1`-`512` (clamped) | `4` | ONNX Runtime intra-op threads. |
| `inter_op_threads` | ORT | `0`-`512` (clamped) | `0` (ONNX Runtime default) | ONNX Runtime inter-op threads. |
| `gpu_mem_limit` | ORT | bytes | unlimited | Memory limit of the CUDA execution provider. |
| `max_concurrent` | TRT | `1`-`32` (clamped) | `1` | Inference slots per model. Each slot has its own execution context, CUDA stream and GPU buffers, so requests run in parallel. More slots: more throughput and more GPU memory. |

Not configurable: the GPU (always device 0, see `CUDA_VISIBLE_DEVICES`), the precision and the graph optimizations (ORT uses all of them, with exhaustive cuDNN algorithm search).

### Models

`ModelConfig` takes either `file_path` or `model_data` + `model_size` (an in-memory buffer; on Windows ORT only supports `file_path`). `ModelConfig.config` is **not used** by either backend: every setting is global. A model's id is its index in the array. Calling `runtime_load_models` again replaces the loaded models.

| | ONNX Runtime | TensorRT |
| --- | --- | --- |
| Model file | `model-simplified.onnx` (any ONNX file works) | `model.trt` |
| Concurrency | One worker thread per model: requests of a model run in order. | `max_concurrent` slots per model share one queue: **outputs can come back in any order**, match them with `Tensors.id`. |
| Input tensors | Matched by name. Dynamic shapes work. | Matched by **position**; the engine has fixed shapes and each input must hold at least the engine's input size. |
| Tensor types | float, float16, double, int8/16/32/64, uint8/16/32/64, bool | float, float16, bfloat16, int8, uint8, int32, int64, bool |
| Type conversion | None: send the type the model declares. | None: send the type the engine declares (an `fp16` engine keeps `float32` inputs and outputs). |

If one request fails (wrong tensor count, bad type, CUDA error), the runtime logs `Inference error` and **returns no output for it**: the host sees a timeout from `runtime_retrieve_output`, so check the log. `runtime_get_info()` returns JSON: ORT `{"loaded_models":N,"requests_in_flight":M}`, TRT also `"total_slots"`. `runtime_get_name()` returns `OAAX NVIDIA OnnxRuntime` or `OAAX NVIDIA TensorRT`.

### Environment variables

The runtime reads none itself. The standard CUDA ones apply: `CUDA_VISIBLE_DEVICES` selects the GPU (set to an empty string, ORT fails at model load with `no CUDA-capable device is detected`), and `LD_LIBRARY_PATH` (Linux) or `PATH` (Windows) is where the ORT runtime finds the unbundled CUDA libraries.

## End-to-end example

This takes the face detection model of the [Hello World example](Getting%20Started.md) from ONNX to detections on an NVIDIA GPU, with the OAAX C example as host application.

!!! note "Prerequisites"
    - Linux x86_64 (Ubuntu 22.04+), an NVIDIA GPU with a driver for CUDA 13 (580+; `nvidia-smi` works), Docker, [uv](https://docs.astral.sh/uv/getting-started/installation/).
    - Build tools: `sudo apt-get install -y git curl cmake build-essential libjpeg-dev zip`.
    - ONNX Runtime only: the CUDA 13 libraries, for example `sudo apt-get install -y cuda-cudart-13-0 libcublas-13-0 libcufft-13-0` from [NVIDIA's CUDA apt repository](https://developer.nvidia.com/cuda-downloads){:target="_blank"}. Check with `ldconfig -p | grep libcublasLt.so.13`.
    - TensorRT only: the [NVIDIA Container Toolkit](https://docs.nvidia.com/datacenter/cloud-native/container-toolkit/latest/install-guide.html){:target="_blank"}; check with `docker run --rm --gpus all ubuntu nvidia-smi`.

**1. Get the host application and the model** (both backends)

```bash
git clone https://github.com/OAAX-standard/examples.git
cd examples/c-example
mkdir build && (cd build && cmake .. && make)
mkdir -p work/model && cp artifacts/model.onnx work/model/ && cd work
```

=== "ONNX Runtime"

    **2. Download** the runtime for CUDA 13 and the toolchain (about 1.8 GB), and load the image:

    ```bash
    uvx oaax-manager fetch runtime nvidia-ort --variant cuda13
    uvx oaax-manager fetch toolchain nvidia-ort
    docker load -i oaax-nvidia-onnxruntime-toolchain-2.0.0.tar
    ```

    **3. Convert** (about 1 s):

    ```bash
    docker run --rm --user $(id -u):$(id -g) -v ./model:/model oaax-nvidia-toolchain:2.0.0 /model/model.onnx /model/ort-output
    ```

    **4. Run**:

    ```bash
    ../build/c_example runtime-nvidia-ort-2.0.0-linux-x86_64-cuda13/libRuntimeLibrary.so model/ort-output/model-simplified.onnx ../artifacts/image.jpg
    ```

    ```text
    Runtime: OAAX NVIDIA OnnxRuntime 2.0.0
    Runtime info: {"loaded_models":1,"requests_in_flight":0}
    Output tensor 'bboxes-format:xyxysc;0:face': type 1, shape [12, 6]
      face 1.000 at (0.651, 0.045) - (0.771, 0.297)
      ...
    OK: 10/10 requests answered in 357 ms (35.7 ms per request)
    ```

    The 12 detections are the 12 faces of the image. The first run is slow because CUDA and cuDNN warm up; the time varies with the GPU. ONNX Runtime may print warnings such as `Expected shape from model of {20,6} does not match actual shape of {12,6}`: the model's number of detections is dynamic, ignore them.

=== "TensorRT"

    !!! warning "The Hello World model cannot run on TensorRT"
        Its `NonMaxSuppression` operator gives the output a data-dependent size (see [Fixed output sizes only](#conversion-toolchain)). Loading its engine fails with `cudaMalloc failed for tensor: bboxes-format:xyxysc;0:face`. The steps below therefore use the same model **cut before the NMS** (two fixed-size outputs, `boxes` and `scores`), which shows the whole TensorRT path but prints no detections.

    **2. Download** the TensorRT 10 runtime for CUDA 13 and the toolchain (11 GB, a few minutes), and load the image (about 1 min):

    ```bash
    uvx oaax-manager fetch runtime nvidia-trt --variant ubuntu22.04-trt10-cuda13
    uvx oaax-manager fetch toolchain nvidia-trt
    docker load -i oaax-nvidia-tensorrt-toolchain-2.0.0.tar
    ```

    **3. Cut the model** before the NMS and pack it with a `config.json`:

    ```bash
    mkdir -p trt-input trt-output
    uv run --no-project --with onnx python -I -c "import onnx.utils as u; u.extract_model('model/model.onnx', 'trt-input/model.onnx', ['image-', 'nms_sensitivity-'], ['boxes', 'scores'])"
    echo '{"precision": "fp16", "workspace_gb": 4}' > trt-input/config.json
    (cd trt-input && zip bundle.zip model.onnx config.json)
    ```

    **4. Convert** on the GPU the model will run on (about 1 min; the engine is 1 MB):

    ```bash
    docker run --rm --gpus all --user $(id -u):$(id -g) -v ./trt-input:/input -v ./trt-output:/output oaax-nvidia-tensorrt-toolchain:2.0.0 /input/bundle.zip /output
    ```

    **5. Run**:

    ```bash
    ../build/c_example runtime-nvidia-trt-2.0.0-linux-x86_64-ubuntu22.04-trt10-cuda13/libRuntimeLibrary.so trt-output/model.trt ../artifacts/image.jpg
    ```

    ```text
    Runtime: OAAX NVIDIA TensorRT 2.0.0
    Runtime info: {"loaded_models":1,"total_slots":1,"requests_in_flight":0}
    Output tensor 'boxes': type 1, shape [1, 4420, 4]
    Output tensor 'scores': type 1, shape [1, 4420, 2]
    OK: 10/10 requests answered in 24.4 ms (2.4 ms per request)
    ```

The C example sets only `log_level` and `log_file` (`runtime.log` in the current directory); to try the [config keys](#runtime_init-config-keys) edit `init_keys` / `init_values` in `src/main.c` and rebuild. The program exits with a non-zero code if a request fails.

## Troubleshooting

Read `runtime.log` (or run with `log_level` `1` and `log_stdout` `true`) first: every failure below is logged there. Each `runtime_*` function also returns a non-zero status and `runtime_get_error()` returns the message.

| Symptom | Cause and fix |
| --- | --- |
| `runtime_init failed: ... Failed to load library libonnxruntime_providers_cuda.so with error: libcublasLt.so.12: cannot open shared object file` (ORT) | The CUDA libraries of the package's major version are not installed (or not on the library path). Install them, or fetch the package for the CUDA version you have (`ldconfig -p \| grep libcublasLt`). The same message names `libcudart`, `libcublas` or `libcufft` when one of those is the missing one. |
| `... libcudnn.so.8: cannot open shared object file` with the `cuda12` ORT package on Linux x86_64 | Known issue of 2.0.0: see the [warning in Requirements](#software). Place cuDNN 8.9.7.29 for CUDA 12 next to `libRuntimeLibrary.so` with the three commands given there. Copying only `libcudnn.so.8` is not enough: the host aborts with `Could not load library libcudnn_ops_infer.so.8`. |
| `CUDA failure 100: no CUDA-capable device is detected` at `runtime_load_models` (ORT); `No CUDA driver or GPU detected` at init | The process sees no GPU: driver not loaded, empty `CUDA_VISIBLE_DEVICES`, or a container started without `--gpus all`. Check `nvidia-smi` in the same environment. |
| `CUDA driver version is insufficient for CUDA runtime version` | The driver is older than the package's CUDA major ([minimum drivers](#software)). Update the driver or use a package for an older CUDA. |
| `nvidia-smi` says `Failed to initialize NVML: Driver/library version mismatch` | The driver was updated but the old kernel module is still loaded. Reboot (or reload the `nvidia` kernel modules). |
| `runtime_load_models failed: Failed to deserialize TRT engine`, with `[TRT] ... Serialization assertion ... Trying to load an engine created with incompatible serialization version` | The file is not a TensorRT engine (for example the `.onnx` was passed). |
| Same, with `Version tag does not match. Note: Current Version: 243, Serialized Engine Version: 240` | The engine was built by another TensorRT version than the runtime's (here a TensorRT 10 engine in a TensorRT 11 runtime). Convert again with the toolchain whose TensorRT matches your runtime package. |
| The engine loads on one machine, not on another (TRT) | Engines are tied to the GPU architecture, the CPU platform and the TensorRT version they were built with (`MIME type` in `logs.json`). Convert on each target GPU model. |
| `runtime_load_models failed: cudaMalloc failed for tensor: ...` (TRT) | The model has a data-dependent output size (for example a `NonMaxSuppression` operator): see [Fixed output sizes only](#conversion-toolchain). |
| `Cannot open engine file: <path>` (TRT, status `FILE_NOT_FOUND`) | Wrong path to `model.trt`. |
| `runtime_retrieve_output` times out and `runtime.log` has `Inference error` | A request was rejected (for example `Input tensor count mismatch`, or an input type or size the model does not declare). Failed requests return no output. |
| The host exits right after `runtime_init` with `Logger init failed: Failed opening file ...` or `terminate called ... stoi` (TRT) | `log_file` points to an unwritable path, or a numeric key (`log_level`) is not a number. Fix the config. |
| Toolchain: `FileNotFoundError: ... 'nvidia-smi'` (TRT) | The container does not see the GPU. Start it with `--gpus all` and install the [NVIDIA Container Toolkit](https://docs.nvidia.com/datacenter/cloud-native/container-toolkit/latest/install-guide.html){:target="_blank"} (`sudo nvidia-ctk runtime configure --runtime=docker && sudo systemctl restart docker`). Docker's own `could not select device driver "" with capabilities: [[gpu]]` means the toolkit is missing. |
| Toolchain: `PermissionError: ... '/input/model-simplified.onnx'` (ORT) | A single file was mounted. Mount the directory that holds the model. |
| Toolchain: `trtexec failed (exit N)` (TRT) | TensorRT cannot convert the model; read the end of `stderr` in the message and the `trtexec` output in `logs.json`. Try a lower `optimization_level`, a larger `workspace_gb`, or the ONNX Runtime backend. |
| Windows: the DLL does not load | Load it with `LoadLibraryEx(path, NULL, LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR \| LOAD_LIBRARY_SEARCH_DEFAULT_DIRS)` (plain `LoadLibrary` does not search the DLL's own folder), and put the CUDA toolkit `bin` folder on `PATH`. |
