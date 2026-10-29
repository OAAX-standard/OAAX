> **📦 Implementation details**  
> The runtime and toolchain are implemented in the [nvidia-acceleration repository](https://github.com/OAAX-standard/nvidia-acceleration){:target="_blank"}. Latest release: [v2.0.0](https://github.com/OAAX-standard/nvidia-acceleration/releases/tag/v2.0.0){:target="_blank"}.

## At a glance

| | |
| --- | --- |
| **Devices** | NVIDIA GPUs (TensorRT toolchain: compute capability 7.5 or higher), Jetson, DGX Spark / Grace |
| **Backends** | ONNX Runtime with the CUDA Execution Provider (`nvidia-ort`), native TensorRT 10 or 11 (`nvidia-trt`) |
| **Runtime library** | `libRuntimeLibrary.so` (Linux), `RuntimeLibrary.dll` (Windows, ONNX Runtime only), 1 MB to 1.7 GB depending on the build |
| **Toolchain image** | `oaax-nvidia-toolchain:2.0.0` (967 MB) or `oaax-nvidia-tensorrt-toolchain:2.0.0` (11 GB) |
| **Start here** | `uvx oaax-manager fetch runtime nvidia-ort --variant cuda13` and `uvx oaax-manager fetch toolchain nvidia-ort`, then the [end-to-end example](#end-to-end-example) |
| **Verified / not** | Verified on an RTX A4000, driver 580, CUDA 13, Ubuntu x86_64: both backends (ONNX Runtime and TensorRT 10), with the YOLOX example. Not verified: Windows, Jetson, DGX Spark / Grace (SBSA), TensorRT 11 conversion, running the CUDA 11 package, the wheel conversion path, `docker run --gpus all` as written |

| **Which backend and `--variant`?** | Backend | `--variant` (Linux x86_64; [other platforms](#which-package-do-i-pick)) |
| --- | --- | --- |
| The simplest path, any model, Windows, or to convert without a GPU | ONNX Runtime (`nvidia-ort`) | `cuda13`, or the CUDA major you have installed (`cuda11`, `cuda12`; check with `ldconfig -p \| grep libcublasLt`) |
| The fastest inference, a model with fixed-size outputs | TensorRT (`nvidia-trt`) | `ubuntu22.04-trt10-cuda13` (TensorRT 10 matches the Docker toolchain) |

## Overview

NVIDIA GPUs are supported through two backends that expose the same OAAX 2.0 runtime interface, so the host application does not change. Use the runtime and the toolchain of the **same** backend together.

| | ONNX Runtime backend | TensorRT backend |
| --- | --- | --- |
| Platforms | Linux x86_64, Jetson, SBSA, **Windows x86_64** | Linux x86_64, Jetson, SBSA (same GPU classes) |
| Toolchain input | `.onnx` file (no GPU needed) | `.zip` bundle: `model.onnx` + `config.json` (needs the target GPU) |
| Toolchain output | `model-simplified.onnx` | `model.trt` engine, built for the GPU it ran on |
| Runtime accepts | `.onnx` path or in-memory buffer; bundles ONNX Runtime and cuDNN | `.trt` path or in-memory buffer; bundles TensorRT and `libcudart` |

- **ONNX Runtime** (start here): the toolchain only simplifies the graph, so you can convert on any machine, and the runtime executes the model with the [CUDA Execution Provider](https://onnxruntime.ai/docs/execution-providers/CUDA-ExecutionProvider.html){:target="_blank"}. Broadest model coverage and the only backend on Windows.
- **TensorRT**: the toolchain compiles an engine for the exact GPU in the machine (`trtexec`), and the runtime loads it directly. Best inference performance, but the engine only runs on the GPU architecture, CPU platform and TensorRT version it was built for.

## Requirements

### Hardware

| Item | Requirement |
| --- | --- |
| GPU | An NVIDIA GPU. The TensorRT toolchain refuses GPUs below compute capability 7.5 (Turing). Verified on an RTX A4000 (8.6). Both runtimes always use GPU 0: pick another with `CUDA_VISIBLE_DEVICES`. |
| GPU memory | Depends on the model. ORT: the CUDA memory arena grows on demand (cap it with `gpu_mem_limit`). TRT: the engine plus one set of input/output buffers per concurrent slot (`max_concurrent`). |
| Jetson / DGX Spark, Grace (`sbsa`) | Jetson, ORT: JetPack 5 (CUDA 11), 6 (CUDA 12) or 7 (CUDA 13); TRT: JetPack 6 only (CUDA 12, TensorRT 10). SBSA, ORT: CUDA 13; TRT: Ubuntu 24.04, TensorRT 10 (CUDA 12 or 13) or TensorRT 11 (CUDA 13). |

### Software

| | Needed to **run** (runtime) | Needed to **convert** (toolchain) |
| --- | --- | --- |
| Tools | `libgomp1` (needed by both runtimes). | `docker`; TRT also `zip`. To fetch and try the example: [`uv`](https://docs.astral.sh/uv/getting-started/installation/){:target="_blank"}, `git`, `curl`, `cmake` and `build-essential` (minimal images lack them: `sudo apt-get install -y git curl cmake build-essential libgomp1 zip`). |
| OS | Linux: Ubuntu 20.04+ for CUDA 11/12 ORT builds; Ubuntu 22.04+ for the CUDA 13 ORT builds and all TRT builds (glibc 2.29 / 2.34 / 2.33). Windows 10/11 (ORT only). | Docker on any Linux x86_64 host (ORT). Docker plus the NVIDIA Container Toolkit, or the Python wheel with a local TensorRT (TRT). |
| NVIDIA driver | New enough for the package's CUDA major version, per [NVIDIA's compatibility table](https://docs.nvidia.com/deploy/cuda-compatibility/){:target="_blank"}: CUDA 11 needs 450.80+, CUDA 12 needs 525.60+, CUDA 13 needs 580+. | TRT only: a driver for CUDA 13 (580+), with `nvidia-smi` working on the host. |
| CUDA libraries | **ORT: not bundled.** Install the CUDA major of your package: `libcudart`, `libcublas`, `libcublasLt` and `libcufft` (Windows: on `PATH`). **TRT: bundled** (`libcudart` and TensorRT), nothing to install. | None on the host (they are inside the image). |
| cuDNN | ORT: bundled, except on Jetson (JetPack provides it) and for the `cuda12` x86_64 package (see the warning below). TRT: not used. | None |
| TensorRT | TRT: bundled (10.16 or 11.0); an engine only loads in a runtime with the **same TensorRT version** as the toolchain that built it. **Jetson: not bundled**, JetPack provides it. | The image contains TensorRT 10.16, so use a `trt10` runtime with it ([issue #22](https://github.com/OAAX-standard/nvidia-acceleration/issues/22){:target="_blank"}). The TRT 11 runtime needs the wheel with TensorRT 11 installed. |

!!! warning "Known issue: the `cuda12` ONNX Runtime package (Linux x86_64) needs cuDNN 8"
    The 2.0.0 `cuda12` package contains cuDNN 9 files, but its ONNX Runtime (1.16.3) loads **cuDNN 8** (`libcudnn.so.8`), so it fails with `libcudnn.so.8: cannot open shared object file`. Download cuDNN **8.9.7.29 for CUDA 12** from NVIDIA and place it **next to `libRuntimeLibrary.so`** (about 786 MB of libraries):

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

## Get the runtime and toolchain

[`oaax-manager`](https://pypi.org/project/oaax-manager/) downloads, verifies (sha256) and unpacks the right build for the machine it runs on. All commands are run with [uv](https://docs.astral.sh/uv/getting-started/installation/):

```bash
uvx oaax-manager fetch runtime nvidia-ort --variant cuda13   # -> runtime-nvidia-ort-2.0.0-linux-x86_64-cuda13/
uvx oaax-manager fetch toolchain nvidia-ort                  # -> oaax-nvidia-onnxruntime-toolchain-2.0.0.tar
docker load -i oaax-nvidia-onnxruntime-toolchain-2.0.0.tar   # loads oaax-nvidia-toolchain:2.0.0

uvx oaax-manager fetch runtime nvidia-trt --variant ubuntu22.04-trt10-cuda13
uvx oaax-manager fetch toolchain nvidia-trt                  # 11 GB
docker load -i oaax-nvidia-tensorrt-toolchain-2.0.0.tar      # loads oaax-nvidia-tensorrt-toolchain:2.0.0
```

### Download links and compatibility matrix

Runtime libraries are packaged as `library-*.tar.gz` and contain `libRuntimeLibrary.so` (`RuntimeLibrary.dll` on Windows) with the libraries it needs. Sizes are in brackets. The sha256 of every file is in the [release notes](https://github.com/OAAX-standard/nvidia-acceleration/releases/tag/v2.0.0){:target="_blank"}. `uvx oaax-manager list` shows every variant (`*` marks the one for your machine). Use `--os windows` or `--arch aarch64` to fetch for another platform. Sizes range from 1 MB (Jetson TensorRT) to 11 GB (TensorRT toolchain).

#### OAAX 2.0: ONNX Runtime backend

Conversion toolchain (any platform): [Download](https://nbg1.your-objectstorage.com/oaax/conversion-toolchain/2.0.0/NVIDIA/oaax-nvidia-onnxruntime-toolchain.tar) (967 MB)

??? note "All 10 ONNX Runtime runtime builds"
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

??? note "All 8 TensorRT runtime builds"
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
    docker run --rm --user "$(id -u):$(id -g)" -v "$PWD/model:/model" oaax-nvidia-toolchain:2.0.0 /model/model.onnx /model/output
    ```

    The arguments are `<onnx-path>` (inside the container) and `<output-directory>` (created if missing; receives `model-simplified.onnx`, named `<input name>-simplified.onnx`, and `logs.json`). There is **no configuration file and no option**. If simplification fails the original model is passed through unchanged and the failure is logged (`Failed to simplify ONNX model`, with `Error` and `Fallback`).

    **Mount the directory, not the file:** the simplified model is first written next to the input, then copied to the output directory, so the input directory must be writable. With `--user` and a single-file mount (`-v "$PWD/model.onnx:/input/model.onnx"`) it fails with `PermissionError: ... '/input/model-simplified.onnx'`. The copy left next to your input can be deleted.

    `logs.json` is a list of `{"Message": ..., "Data": {...}}` entries: `Simplifying ONNX model` (`ONNX Path`, `MD5`), then `Simplified ONNX model successfully` or the failure above. The final `Successful Conversion` entry (`Output file name`, `Output file MD5`, `MIME type` = `application/x-onnx; device=cpu`) is only printed to the console.

=== "TensorRT"
    Needs the GPU the model will run on (the engine is tuned for it). Pack the model and a `config.json` into a zip:

    ```bash
    mkdir -p trt-input trt-output && cp model.onnx trt-input/
    echo '{"precision": "fp16", "workspace_gb": 4}' > trt-input/config.json
    (cd trt-input && zip bundle.zip model.onnx config.json)

    docker run --rm --gpus all --user "$(id -u):$(id -g)" -v "$PWD/trt-input:/input" -v "$PWD/trt-output:/output" \
      oaax-nvidia-tensorrt-toolchain:2.0.0 /input/bundle.zip /output
    ```

    ??? note "All `config.json` options (most users only set `precision` and `workspace_gb`)"
        | Key | Required | Values | Default | Effect |
        | --- | --- | --- | --- | --- |
        | `precision` | yes | `fp32`, `fp16`, `int8` | | Engine precision. |
        | `workspace_gb` | yes | positive number | | Builder workspace limit (`trtexec --memPoolSize=workspace:<n>g`). |
        | `optimization_level` | no | integer `1`-`5` | `3` (TensorRT) | Builder effort (`--builderOptimizationLevel`): higher is slower to build, can be faster to run. |
        | `avg_timing` | no | integer >= 1 | `8` (TensorRT) | Timing iterations per kernel choice (`--avgTiming`). |
        | `calibration_data` | int8 only | folder in the zip, e.g. `"calib/"` | | Representative images (jpg, jpeg, png, bmp, webp); 100-500 are recommended. |
        | `preprocessing` | no | `{"mean": [r,g,b], "std": [r,g,b]}` | ImageNet mean/std | int8 only. Images are resized (bilinear) to the model's input size, scaled to 0-1, then normalized. The first model input must be 4-D `NCHW`. |

    **Outputs** in `/output`: `model.trt` and `logs.json` (plus `model.trt.calib`, the calibration cache, for int8 on TensorRT 10). `logs.json` holds the same `Message`/`Data` entries as above: the detected GPU (`sm_86`), the input MD5 and config, the exact `trtexec` command with the tail of its output, and `Successful Conversion` with the output MD5 and a MIME type such as `application/x-tensorrt; arch=x86_64; sm=86; trt=10` (CPU architecture, GPU compute capability, TensorRT major).

    How the engine is built: `fp32` with `trtexec`; `fp16` with `trtexec --fp16` on TensorRT 10, or on TensorRT 11 (wheel) by first converting the ONNX model to float16 with `onnxconverter-common`, keeping float32 inputs and outputs; `int8` on TensorRT 10 by entropy calibration with your images (needs the `tensorrt` and `pycuda` Python packages), on TensorRT 11 with Q/DQ nodes inserted by `nvidia-modelopt`.

    `optimization_level` and `avg_timing` only apply to builds through `trtexec` (fp32, fp16, and int8 on TensorRT 11). The int8 build of TensorRT 10 ignores them.

    !!! warning "Fixed output sizes only"
        The TensorRT runtime allocates its buffers from the engine's tensor shapes. A model whose output size depends on the data, typically one with a `NonMaxSuppression` operator inside the graph, converts without error but the engine has a dynamic output dimension (`-1`) and fails to load: `runtime_load_models failed: cudaMalloc failed for tensor: <name>`. Use the ONNX Runtime backend for such models, or move the NMS out of the model, as the YOLOX example does ([issue #19](https://github.com/OAAX-standard/nvidia-acceleration/issues/19){:target="_blank"}).

    **Without Docker** (for example where containers cannot see the GPU): install the TensorRT SDK so `trtexec` is on `PATH`, then run the [wheel](https://nbg1.your-objectstorage.com/oaax/conversion-toolchain/2.0.0/NVIDIA/oaax_nvidia_tensorrt_conversion-2.0.0-py3-none-any.whl). For int8, install the matching extra: `[int8-trt10]` or `[int8-trt11]`.

    !!! warning "Check `nvidia-smi` first"
        The wheel runs `nvidia-smi` before anything else. If it is not installed you get a Python traceback ending in `FileNotFoundError: ... 'nvidia-smi'`; if it fails (for example after a driver update without a reboot) the traceback ends in `RuntimeError: nvidia-smi failed (exit N)`. Neither says what to do: install the driver, or reboot, until `nvidia-smi` prints your GPU, then retry ([issue #23](https://github.com/OAAX-standard/nvidia-acceleration/issues/23){:target="_blank"}). Only after that does it report missing `trtexec` or int8 packages (`Dependency issues for ...`) and warn when the TensorRT major is not 10 or 11.

    ```bash
    uvx --from ./oaax_nvidia_tensorrt_conversion-2.0.0-py3-none-any.whl conversion_toolchain --input-path bundle.zip --output-dir ./trt-output
    ```

    ??? note "TensorRT toolchain error messages"
        Errors are raised as Python exceptions (non-zero exit status):

        | Message | Cause |
        | --- | --- |
        | `Missing required config key: 'precision'` / `precision must be one of ['fp32', 'fp16', 'int8']` / `workspace_gb must be a positive number` / `optimization_level must be an integer 1-5` / `avg_timing must be a positive integer` | A required key is absent or a value is invalid. |
        | `'calibration_data' is required in config.json when precision is 'int8'` | int8 without calibration images. |
        | `model.onnx not found in zip archive` / `config.json not found in zip archive` | The zip must have both files at its top level. |
        | `Dependency issues for <precision> conversion: ...` | `nvidia-smi`, `trtexec` or an int8 package is missing. |
        | `Detected GPU compute capability ... is not supported by TensorRT 10 (minimum: sm_75 / Turing)` | GPU too old. |
        | `trtexec failed (exit N). stderr: ...` | TensorRT rejected the model (unsupported operator, not enough workspace...). The full output is in `logs.json`. |

## Runtime library

The runtime implements the [OAAX 2.0 runtime interface](Specification.md#oaax-runtime-specification). Load `libRuntimeLibrary.so` from the folder you extracted it to (it finds its bundled libraries next to itself), call `runtime_init`, then `runtime_load_models` with the converted model.

### `runtime_init` config keys

All keys and values are strings. Unknown keys are ignored with a warning in the log. Not configurable: the GPU (always device 0, see `CUDA_VISIBLE_DEVICES`), the precision and the graph optimizations (ORT uses all of them, with exhaustive cuDNN algorithm search).

??? note "All `runtime_init` config keys (most users only set `log_level`, `log_file` and `perf_mode`)"
    | Key | Backend | Values | Default | Effect |
    | --- | --- | --- | --- | --- |
    | `log_level` | both | `0` trace, `1` debug, `2` info, `3` warning, `4` error, `5` critical, `6` off | `2` | Minimum level, for the log file and the console. An out-of-range number falls back to `2`; a non-number is an error (TRT: aborts the host). |
    | `log_file` | both | path | `runtime.log` | Log file, rotated at 5 MB, 3 files kept. On Windows the default is a per-process file in the temp directory. If it cannot be created, ORT logs to the console only; **TRT terminates the host process** ([issue #20](https://github.com/OAAX-standard/nvidia-acceleration/issues/20){:target="_blank"}), so give it a writable path. |
    | `log_stdout` | both | `true` | off | Also log to the console. Only the exact value `true` enables it. |
    | `perf_mode` | both | `power`, `eco` | unset | Picks defaults from the hardware; explicit keys below override it. ORT `power`: threads = CPU cores, no GPU memory cap. ORT `eco`: half the cores, half the GPU memory. TRT `power`: `max_concurrent` = clamp(SM count / 8, 2, 16). TRT `eco`: half of that. Any other value is ignored with a warning. |
    | `num_threads` / `inter_op_threads` / `gpu_mem_limit` | ORT | `1`-`512` (clamped) / `0`-`512` (clamped) / bytes | `4` / `0` (ONNX Runtime default) / unlimited | ONNX Runtime intra-op threads (`num_threads`), inter-op threads (`inter_op_threads`), and the memory limit of the CUDA execution provider (`gpu_mem_limit`). |
    | `max_concurrent` | TRT | `1`-`32` (clamped) | `1` | Inference slots per model. Each slot has its own execution context, CUDA stream and GPU buffers, so requests run in parallel. More slots: more throughput and more GPU memory. |

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

This takes the official YOLOX-S object detector from ONNX to detections on an NVIDIA GPU, with the [YOLOX example](https://github.com/OAAX-standard/examples/tree/main/yolox-example){:target="_blank"} (a small C host that works with every OAAX 2.0 runtime) and a public-domain photo of a dog and a bicycle.

!!! note "Prerequisites"
    - Linux x86_64 (Ubuntu 22.04+), an NVIDIA GPU with a driver for CUDA 13 (580+; `nvidia-smi` prints your GPU), Docker, and [uv](https://docs.astral.sh/uv/getting-started/installation/){:target="_blank"}. Tools: `sudo apt-get install -y git curl cmake build-essential libgomp1 zip`.
    - ONNX Runtime only: the CUDA 13 libraries, which the package does not bundle. On Ubuntu 22.04, add NVIDIA's CUDA apt repository, then install them (use `ubuntu2404` instead of `ubuntu2204` on 24.04):

        ```bash
        curl -fLO https://developer.download.nvidia.com/compute/cuda/repos/ubuntu2204/x86_64/cuda-keyring_1.1-1_all.deb
        sudo dpkg -i cuda-keyring_1.1-1_all.deb && sudo apt-get update
        sudo apt-get install -y cuda-cudart-13-0 libcublas-13-0 libcufft-13-0
        ldconfig -p | grep libcublasLt.so.13      # must print a line
        ```
    - TensorRT only: the [NVIDIA Container Toolkit](https://docs.nvidia.com/datacenter/cloud-native/container-toolkit/latest/install-guide.html){:target="_blank"}, checked with `docker run --rm --gpus all debian:12 nvidia-smi`.

**1. Get the host application and the model** (both backends)

```bash
git clone https://github.com/OAAX-standard/examples.git
cd examples/yolox-example
cmake -S . -B build && cmake --build build      # builds build/yolox_host
bash fetch_model.sh                             # models/yolox_s.onnx (36 MB), sha256 checked
```

=== "ONNX Runtime"
    ```bash
    # 2. Download the CUDA 13 runtime (823 MB) and the toolchain (967 MB), and load the image
    uvx oaax-manager fetch runtime nvidia-ort --variant cuda13
    uvx oaax-manager fetch toolchain nvidia-ort
    docker load -i oaax-nvidia-onnxruntime-toolchain-2.0.0.tar

    # 3. Convert (about 4 s)
    mkdir -p ort && cp models/yolox_s.onnx ort/model.onnx
    docker run --rm --user "$(id -u):$(id -g)" -v "$PWD/ort:/model" oaax-nvidia-toolchain:2.0.0 /model/model.onnx /model/output

    # 4. Run
    ./build/yolox_host runtime-nvidia-ort-2.0.0-linux-x86_64-cuda13/libRuntimeLibrary.so ort/output/model-simplified.onnx artifacts/dog-bicycle.jpg
    ```

    ```text
    Runtime: OAAX NVIDIA OnnxRuntime 2.0.0
    Runtime info: {"loaded_models":1,"requests_in_flight":0}
    Input: 'images' float32 NCHW, 1280x854 image letterboxed to 640
    Output tensor 'output': type 1, shape [1, 8400, 85]
    Detections (score >= 0.30, NMS IoU 0.45): 2
      bicycle        0.953  ( 616,  210) - (1228,  805)
      dog            0.934  ( 216,  244) - ( 521,  587)
    Wrote detections.png
    OK: 10 requests, first 314.8 ms (warm-up), then 11.0 ms on average (min 10.5 ms)
    ```

=== "TensorRT"
    ```bash
    # 2. Download the TensorRT 10 runtime for CUDA 13 (326 MB) and the toolchain (11 GB, a few minutes), and load the image (about 1 min)
    uvx oaax-manager fetch runtime nvidia-trt --variant ubuntu22.04-trt10-cuda13
    uvx oaax-manager fetch toolchain nvidia-trt
    docker load -i oaax-nvidia-tensorrt-toolchain-2.0.0.tar

    # 3. Convert on the GPU the model will run on (about 2 min; the engine is 21 MB); configs/tensorrt.json is { "precision": "fp16", "workspace_gb": 4 }
    mkdir -p trt/input trt/output && cp models/yolox_s.onnx trt/input/model.onnx && cp configs/tensorrt.json trt/input/config.json
    (cd trt/input && zip bundle.zip model.onnx config.json)
    docker run --rm --gpus all --user "$(id -u):$(id -g)" -v "$PWD/trt/input:/input" -v "$PWD/trt/output:/output" \
      oaax-nvidia-tensorrt-toolchain:2.0.0 /input/bundle.zip /output

    # 4. Run
    ./build/yolox_host runtime-nvidia-trt-2.0.0-linux-x86_64-ubuntu22.04-trt10-cuda13/libRuntimeLibrary.so trt/output/model.trt artifacts/dog-bicycle.jpg
    ```

    ```text
    Runtime: OAAX NVIDIA TensorRT 2.0.0
    Runtime info: {"loaded_models":1,"total_slots":1,"requests_in_flight":0}
    ...                                  (same input, output tensor and detections lines as ONNX Runtime)
    OK: 10 requests, first 28.4 ms (warm-up), then 5.4 ms on average (min 5.3 ms)
    ```

`detections.png` is the photo with the two boxes drawn. The same host runs both: only the runtime library and the model file change. The first request is slow because CUDA (and cuDNN or the engine) warm up; times depend on the GPU. Pass runtime options with `--config KEY=VALUE`, for example `--config perf_mode=power` (see the [config keys](#runtime_init-config-keys)). The program exits with a non-zero code if a request fails; the runtime's reason is in `runtime.log`.

## Troubleshooting

Read `runtime.log` (or run with `log_level` `1` and `log_stdout` `true`) first: every failure below is logged there. Each `runtime_*` function also returns a non-zero status and `runtime_get_error()` returns the message.

| Symptom | Cause and fix |
| --- | --- |
| `runtime_init failed: ... Failed to load library libonnxruntime_providers_cuda.so with error: libcublasLt.so.12: cannot open shared object file` (ORT) | The CUDA libraries of the package's major version are not installed (or not on the library path). Install them, or fetch the package for the CUDA version you have (`ldconfig -p \| grep libcublasLt`). The same message names `libcudart`, `libcublas` or `libcufft` when one of those is the missing one. |
| `... libcudnn.so.8: cannot open shared object file` with the `cuda12` ORT package on Linux x86_64 | Known issue of 2.0.0 ([issue #18](https://github.com/OAAX-standard/nvidia-acceleration/issues/18){:target="_blank"}): see the [warning in Requirements](#software). Place cuDNN 8.9.7.29 for CUDA 12 next to `libRuntimeLibrary.so` with the three commands given there. Copying only `libcudnn.so.8` is not enough: the host aborts with `Could not load library libcudnn_ops_infer.so.8`. |
| `CUDA failure 100: no CUDA-capable device is detected` at `runtime_load_models` (ORT); `No CUDA driver or GPU detected` at init | The process sees no GPU: driver not loaded, empty `CUDA_VISIBLE_DEVICES`, or a container started without `--gpus all`. Check `nvidia-smi` in the same environment. |
| `CUDA driver version is insufficient for CUDA runtime version` | The driver is older than the package's CUDA major ([minimum drivers](#software)). Update the driver or use a package for an older CUDA. |
| `nvidia-smi` says `Failed to initialize NVML: Driver/library version mismatch` | The driver was updated but the old kernel module is still loaded. Reboot (or reload the `nvidia` kernel modules). |
| `runtime_load_models failed: Failed to deserialize TRT engine`, with `[TRT] ... Serialization assertion ... Trying to load an engine created with incompatible serialization version` | The file is not a TensorRT engine (for example the `.onnx` was passed). Or, with `Version tag does not match. Note: Current Version: 243, Serialized Engine Version: 240`: the engine was built by another TensorRT version than the runtime's (here a TensorRT 10 engine in a TensorRT 11 runtime); convert again with the toolchain whose TensorRT matches your runtime package. |
| The engine loads on one machine, not on another (TRT) | Engines are tied to the GPU architecture, the CPU platform and the TensorRT version they were built with (`MIME type` in `logs.json`). Convert on each target GPU model. |
| `runtime_load_models failed: cudaMalloc failed for tensor: ...` (TRT) | The model has a data-dependent output size (for example a `NonMaxSuppression` operator): see [Fixed output sizes only](#conversion-toolchain) ([issue #19](https://github.com/OAAX-standard/nvidia-acceleration/issues/19){:target="_blank"}). Models without such outputs, like YOLOX, load fine. |
| ORT prints `Expected shape from model of {20,6} does not match actual shape of {12,6}` on every request | Harmless: it only appears for models with a dynamic output size, and goes to the terminal, not the log ([issue #24](https://github.com/OAAX-standard/nvidia-acceleration/issues/24){:target="_blank"}). The YOLOX example prints none. |
| `Cannot open engine file: <path>` (TRT, status `FILE_NOT_FOUND`) | Wrong path to `model.trt`. |
| `runtime_retrieve_output` times out and `runtime.log` has `Inference error` | A request was rejected (for example `Input tensor count mismatch`, or an input type or size the model does not declare). Failed requests return no output. |
| The host exits right after `runtime_init` with `Logger init failed: Failed opening file ...` or `terminate called ... stoi` (TRT) | `log_file` points to an unwritable path, or a numeric key (`log_level`) is not a number. Fix the config. |
| Toolchain: `FileNotFoundError: ... 'nvidia-smi'` (TRT) | The container does not see the GPU. Start it with `--gpus all` and install the [NVIDIA Container Toolkit](https://docs.nvidia.com/datacenter/cloud-native/container-toolkit/latest/install-guide.html){:target="_blank"} (`sudo nvidia-ctk runtime configure --runtime=docker && sudo systemctl restart docker`). Docker's own `could not select device driver "" with capabilities: [[gpu]]` means the toolkit is missing. |
| Toolchain: `trtexec failed (exit N)` (TRT) | TensorRT cannot convert the model; read the end of `stderr` in the message and the `trtexec` output in `logs.json`. Try a lower `optimization_level`, a larger `workspace_gb`, or the ONNX Runtime backend. |
| Windows: the DLL does not load | Load it with `LoadLibraryEx(path, NULL, LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR \| LOAD_LIBRARY_SEARCH_DEFAULT_DIRS)` (plain `LoadLibrary` does not search the DLL's own folder), and put the CUDA toolkit `bin` folder on `PATH`. |
