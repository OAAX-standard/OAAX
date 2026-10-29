> **📦 Implementation details**  
> You can find the implementation of the Intel CPU, GPU and NPU runtime & toolchain in the [Github repository](https://github.com/OAAX-standard/intel-acceleration){:target="_blank"}, and its latest release in [v2.0.0](https://github.com/OAAX-standard/intel-acceleration/releases/tag/v2.0.0){:target="_blank"}.

## At a glance

| | |
| --- | --- |
| **Devices** | Intel CPU, integrated or discrete GPU (Arc, Iris Xe, UHD, Data Center GPU) and NPU, one runtime for all three |
| **Engine** | [OpenVINO™](https://docs.openvino.ai/2026/index.html) 2026.1.0 (native C++ API) |
| **Toolchain input** | `.zip` bundle with `model.onnx` and an optional `config.json` (plus calibration images for INT8) |
| **Toolchain output** | `<model>.zip` with the OpenVINO IR (`model.xml` and `model.bin`), plus `logs.json` |
| **Toolchain image** | `oaax-intel-toolchain:latest` (Docker, x86_64, about 1.2 GB) |
| **Runtime library** | `libRuntimeLibrary.so` (Linux x86_64) or `RuntimeLibrary.dll` (Windows x86_64), shipped with the OpenVINO libraries |
| **Device choice** | `device_type` = `CPU` (default), `GPU`, `NPU` or `AUTO`, global or per model |

## Overview

The toolchain converts an ONNX model to OpenVINO IR. The runtime loads that IR and runs it on the Intel device you select.

- **Toolchain:** by default it compresses the weights to FP16 **and** turns every model input into FP16. Optionally it quantizes to INT8 (NNCF) or bakes input conversion, mean and scale into the model. It does not change the model's operators or outputs.
- **Runtime:** it loads one or more `.zip` IR bundles, runs requests asynchronously and returns outputs tagged with the request id. It accepts an input tensor only if its data type is **exactly** the IR input's type (no conversion), and any shape the IR allows (batch 1 by default, a fixed batch or a dynamic batch if you convert it so).
- **Devices:** the runtime asks OpenVINO for the device; if compiling the model on a non-CPU device fails, it falls back to the CPU and says so only in `runtime.log` and in `runtime_get_info` (`effective_device`).

## Requirements

### Hardware

| Device | Supported Intel hardware ([OpenVINO system requirements](https://docs.openvino.ai/2026/about-openvino/release-notes-openvino/system-requirements.html)) | Checked by us |
| --- | --- | --- |
| CPU | Core 6th to 14th gen and Core Ultra, Xeon, Atom. The runtime refuses to start without **AVX2** | Core i7-12700K |
| GPU | Arc, Iris Xe, UHD / HD Graphics, Data Center GPU Flex and Max | UHD Graphics 770 (integrated) |
| NPU | Core Ultra NPU | not verified on hardware |

### Software

**To run a converted model:**

| | Linux | Windows |
| --- | --- | --- |
| OS | Ubuntu 22.04 or higher, Debian 12 or higher (needs glibc 2.34 and libstdc++ 3.4.30) | Windows 10, 11 (x86_64) (not verified) |
| System packages | `libgomp1`. Minimal images (containers) do not have it | none, the Visual C++ runtime DLLs are in the archive |
| OpenVINO | included in the runtime archive (2026.1.0), nothing to install | included |
| GPU | an OpenCL GPU driver, see [Installation](#installation) | Intel graphics driver |
| NPU | the NPU driver, see [Installation](#installation) | NPU driver |

**To convert a model:** Docker 20.10 or higher on an x86_64 host and 1.2 GB of disk for the image. Nothing else: Python, OpenVINO (2026.4.1 in the image) and NNCF 2.19 are inside the image.

### Installation

Only GPU and NPU need drivers. The CPU needs none.

=== "GPU (Ubuntu)"

    ```bash
    sudo apt-get install -y ocl-icd-libopencl1 intel-opencl-icd
    sudo usermod -aG render $USER     # log out and in again
    ```

    This is what we verified (Ubuntu 22.04, integrated GPU). For newer GPUs use Intel's current driver packages, see the [OpenVINO GPU guide](https://docs.openvino.ai/2026/get-started/install-openvino/configurations/configurations-intel-gpu.html){:target="_blank"}. Discrete Arc GPUs want kernel 6.2 or higher.

=== "NPU (Ubuntu)"

    Install the driver from [intel/linux-npu-driver](https://github.com/intel/linux-npu-driver){:target="_blank"}; it needs Ubuntu 22.04 or higher with kernel 6.6 or higher. See the [OpenVINO NPU guide](https://docs.openvino.ai/2026/get-started/install-openvino/configurations/configurations-intel-npu.html){:target="_blank"}.

=== "Windows"

    Install the current Intel graphics driver (GPU) or NPU driver, see the two guides above.

!!! tip "Check what the runtime sees"
    The `available_devices` field of `runtime_get_info()` (and the `Available devices` lines of `runtime.log`) list the devices OpenVINO found. If `GPU` is missing, the driver or the `render` group is the problem.

!!! warning "`GPU` means any OpenCL GPU"
    Without the Intel driver, a host with another vendor's OpenCL GPU lists that GPU as `GPU`, and the runtime uses it. With several GPUs, `GPU` becomes `MULTI:GPU.0,GPU.1,...`; use `GPU.0`, `GPU.1` to pick one.

## Get the runtime and toolchain

With [`oaax-manager`](https://pypi.org/project/oaax-manager/) (needs [uv](https://docs.astral.sh/uv/getting-started/installation/)); it verifies the sha256 of every download. The vendor is `intel`; there are no variants.

```bash
uvx oaax-manager list                    # the Intel rows: runtime linux, runtime windows, toolchain
uvx oaax-manager fetch runtime intel     # writes runtime-intel-2.0.0-linux-x86_64/ (all 31 files)
uvx oaax-manager fetch toolchain intel   # saves oaax-intel-toolchain-2.0.0.tar
docker load -i oaax-intel-toolchain-2.0.0.tar
uvx oaax-manager fetch runtime intel --os windows   # the Windows runtime
```

### Download links and compatibility matrix

Runtime libraries are packaged as `library.tar.gz`. Extract all the files into the same directory: the runtime finds the OpenVINO libraries next to itself, no `LD_LIBRARY_PATH` needed. The sha256 of each file is in the [v2.0.0 release notes](https://github.com/OAAX-standard/intel-acceleration/releases/tag/v2.0.0).

| OAAX version | OS      | Version         | CPU architecture | Runtime library | Conversion toolchain |
| ------------ | ------- | --------------- | ---------------- | --------------- | -------------------- |
| 2.0.0        | Ubuntu  | 22.04 or higher | x86_64           | [Download](https://nbg1.your-objectstorage.com/oaax/runtimes/2.0.0/INTEL/x86_64/Ubuntu/library.tar.gz) (90 MB) | [Download](https://nbg1.your-objectstorage.com/oaax/conversion-toolchain/2.0.0/INTEL/oaax-intel-toolchain.tar) (1.2 GB) |
| 2.0.0        | Windows | 10, 11          | x86_64           | [Download](https://nbg1.your-objectstorage.com/oaax/runtimes/2.0.0/INTEL/x86_64/Windows/library.tar.gz) (61 MB) | ⬆️ |

## Conversion toolchain

The image has the tag `oaax-intel-toolchain:latest` only (there is no `:2.0.0` tag; the version is in the image label `version=2.0.0`). Its entrypoint is `conversion_toolchain INPUT_ZIP OUTPUT_DIR`. The container runs as root by default, so pass your own user to own the output files:

```bash
mkdir -p input output
cp model.onnx input/ && (cd input && zip model.zip model.onnx)
docker run --rm --user $(id -u):$(id -g) -v "$PWD/input":/input -v "$PWD/output":/output \
  oaax-intel-toolchain:latest /input/model.zip /output
```

!!! note "Bundle layout"
    `model.onnx` (any `*.onnx` name; if there are several, the first is used and a warning is logged) and `config.json` must be at the **root** of the zip, not in a folder. The output is named after the ONNX file: `model.onnx` gives `model.zip`.

### `config.json` options

Every key is optional. Unknown keys are ignored. Example: `{ "optimization": { "fp16_compression": false } }`.

| Key | Type, default | Effect |
| --- | --- | --- |
| `optimization.fp16_compression` | bool, `true` | Stores the weights in FP16 (half the size) **and**, unless `preprocessing.input_dtype` is set, makes every input FP16. `false` keeps the model's input types (for example FP32). Forced to `false` when quantizing |
| `optimization.quantization.enabled` | bool, `false` | INT8 post-training quantization (NNCF). Needs calibration images; makes the input `u8` with `/255` baked in |
| `optimization.quantization.preset` | `mixed`, `performance`, `accuracy`; `mixed` | `performance` quantizes all layers (fastest). `accuracy` is accepted but gives the same IR as `mixed` |
| `optimization.quantization.subset_size` | int > 0, `300` | Maximum number of calibration images used |
| `optimization.quantization.target_device` | `any`, `cpu`, `gpu`, `npu`; `any` | NNCF target. `npu` forces `performance` and keeps attention and DFL MatMuls in FP16 |
| `optimization.quantization.mode`, `.calibration_data` | `int8`, `"calibration/"` | Accepted, no effect: INT8 is always used, the folder is found by name (below) |
| `preprocessing.input_dtype` | `u8`, `f16`, `f32` or `null` | Element type of **all** inputs; the IR converts back to the model's type inside |
| `preprocessing.mean_values`, `.scale_values` | list of numbers or `null` | `(x - mean) / scale` per channel, baked in and applied to **every** input (rank 4 is read as NCHW, rank 3 as CHW) |
| `advanced.batch_size` | int > 0, `1` | Fixes the batch dimension. Fails if the model has batch 1 hard-coded in its post-processing |
| `advanced.dynamic_batch` | bool, `false` | Makes the batch dimension dynamic (`-1`). Excludes `batch_size` > 1 |
| `model_file`, `advanced.input_shape`, `.input_names`, `.output_names` | | Accepted, no effect (not implemented in 2.0.0) |

**Input type of the converted model:**

| Config | Input type in the IR |
| --- | --- |
| default | `f16` |
| `fp16_compression: false` | unchanged (usually `f32`) |
| quantization enabled | `u8` |
| `preprocessing.input_dtype` set | the value you set |

**INT8:** add `"quantization": {"enabled": true}` under `optimization` and put JPEG, PNG, BMP or TIFF images in `calibration/` (`images/` or `data/` also work) in the zip: `zip -r model.zip model.onnx config.json calibration/`. Images are resized to the model's height and width. Calibration assumes a **single image input**.

### Output and errors

`<model>.zip` holds `model.xml` and `model.bin`. `logs.json` is a JSON array of `{Timestamp, Message, Data}` steps; the last one is `Successful Conversion` (output name, MD5, optimizations applied) or `FATAL ERROR: ...` with the error. The log is also printed. Exit codes and the messages we triggered:

| Exit | Meaning | Message seen |
| --- | --- | --- |
| 0 | Success | |
| 1 | Input not found | `Input zip file does not exist: /input/x.zip` |
| 2 | Invalid input or config | `Invalid zip archive: File is not a zip file` (a bare `.onnx` was passed); `No ONNX model (*.onnx) found in input archive` (model missing or in a folder); `Invalid quantization preset: ...`; `Invalid batch_size: 0 ...`; `dynamic_batch=true and batch_size > 1 are mutually exclusive`; `Quantization is enabled but no calibration data provided ...`; malformed JSON, e.g. `Expecting ',' delimiter ...` |
| 3 | Conversion or quantization failed | OpenVINO error text. Example: INT8 on a model with a second, non-image input fails with `Layout [...] doesn't have channels dimension`, because the `u8` and `/255` step is applied to every input |
| 4 | I/O error (permissions, disk full) | |
| 255 | Unexpected error | |

## Runtime library

The runtime implements the [OAAX runtime interface](Specification.md#oaax-runtime-specification). `runtime_init(Config)` takes string key/value pairs; unknown keys are ignored with a warning in the log. Invalid values fall back to the default.

### `runtime_init` keys

| Key | Values, default | Meaning |
| --- | --- | --- |
| `device_type` | `CPU` (default), `GPU`, `NPU`, `AUTO`, or any OpenVINO device name (`GPU.1`) | Device used by the models. `GPU` with several GPUs becomes `MULTI:GPU.0,GPU.1,...`. `AUTO` starts on the CPU and moves to an accelerator when ready. A device that does not exist or fails to compile falls back to the CPU |
| `perf_hint` | `latency` (default), `throughput`, `cumulative_throughput` | OpenVINO performance mode. Use `cumulative_throughput` with `MULTI`. Other values mean `latency` |
| `num_requests` | int, `0` = auto | Parallel inference requests per model. Auto asks OpenVINO (we saw 1 with `latency`, 4 to 5 with `throughput`) |
| `num_streams` | int, `0` = auto | OpenVINO streams, only used with `throughput`; 4 is recommended for a discrete GPU |
| `auto_batch_size` | int, `0` = off | Wraps the device in `BATCH:<device>(N)` to batch concurrent requests. Ignored on the CPU |
| `npu_turbo` | `1` or `0` (default) | Sets `NPU_TURBO=YES` on an NPU (not verified on hardware) |
| `cache_dir` | path, `.` (default), `""` = off | OpenVINO compiled-model cache (`<hash>.blob` files), shared by all models. It avoids recompiling at every start and is **created in the current directory by default** |
| `max_queue_size` | int, `100`, `0` = unlimited | Pending inputs per model and pending outputs. When full, `runtime_enqueue_input` returns `RUNTIME_STATUS_ERROR` (`input queue at capacity`) |
| `log_level` | `0` trace to `6` off, `2` (default) | Minimum log level |
| `log_file` | path, `runtime.log` | Log file, rotated at 5 MB. Each start begins a fresh file and keeps the last three as `runtime.1.log` to `runtime.3.log`. If it cannot be opened, the runtime logs to the console |
| `log_stdout` | `1` or `0` (default) | Also log to the console |

### Per-model keys

`ModelConfig.config` accepts `device_type`, `perf_hint` and `num_requests`, which override the global value for that model, for example to run a detector on the GPU and a classifier on the CPU. `cache_dir` is accepted there but has **no effect**: the cache directory is global. The other keys are global only.

### Behavior

- **Models:** `ModelConfig.file_path` must be the `.zip` from the toolchain (a `.onnx` or `.xml` fails with `Failed to extract model archive`). It is unpacked into the temp directory (`TMPDIR`) and removed at cleanup. Load all models in one `runtime_load_models` call; a second call returns `ALREADY_INITIALIZED` until `runtime_cleanup()`. The model id is its index.
- **Data types:** the input type must equal the IR's, otherwise `runtime_enqueue_input` returns `RUNTIME_STATUS_INVALID_ARGUMENT` with `dtype mismatch for input 'x': model expects f16 but caller sent f32`. Supported types: float, float16, bfloat16, double, int8 to int64, uint8 to uint64, bool, string, int4, uint4.
- **Output shapes:** outputs have the IR's data type. An output with a dynamic dimension (for example the number of detections) has the exact size on the CPU, but on the GPU we saw it padded to its upper bound with zero rows.
- **Shapes:** a wrong shape or an unknown input name is **not** reported by `runtime_enqueue_input` (see [Troubleshooting](#troubleshooting)).
- **Concurrency:** each model has its own pool of inference requests and a queue; all outputs go to one queue, in completion order, so match them by `Tensors.id`. Enqueue and retrieve can be called from different threads. Keep at least `num_requests` inputs in flight to keep a fast model busy.
- **Diagnostics:** `runtime_get_info()` returns JSON: `loaded_models`, `requests_in_flight`, `output_queue_depth`, `backend_version`, `perf_hint`, `max_queue_size`, `available_devices` and per model `effective_device`, `num_infer_requests`, `input_queue_depth`, `inputs` (name, dtype, shape, `-1` = dynamic) and `outputs`.
- **Environment variables:** none are required. `OAAX_DEBUG_CORE_TYPE` (`ANY`, `PCORE`, `ECORE`) forces the CPU core type; it is for diagnostics only.

> You can check out the examples repository for more details on how to use the runtime library: [OAAX Examples](https://github.com/OAAX-standard/examples).

## End-to-end example

The [C example](https://github.com/OAAX-standard/examples/tree/main/c-example) with its face detection model and image, from nothing to detections. Checked on Ubuntu 22.04.

!!! note "Prerequisites"
    `sudo apt-get install -y git curl cmake build-essential libjpeg-dev zip libgomp1`, [Docker](https://docs.docker.com/engine/install/) and [uv](https://docs.astral.sh/uv/getting-started/installation/). For the GPU or NPU tab, the drivers from [Installation](#installation).

1. Download the runtime (90 MB, 3 s) and the toolchain (1.2 GB, 25 s) and load the image:

    ```bash
    mkdir oaax-intel && cd oaax-intel
    uvx oaax-manager fetch runtime intel
    uvx oaax-manager fetch toolchain intel
    docker load -i oaax-intel-toolchain-2.0.0.tar
    ```

2. Build the host application. The `sed` line makes it read the device from the `DEVICE` environment variable (it passes `device_type`):

    ```bash
    git clone https://github.com/OAAX-standard/examples.git
    sed -i 's|{"log_level", "log_file"}|{"log_level", "log_file", "device_type"}|; s|{"3", "runtime.log"}|{"3", "runtime.log", getenv("DEVICE") ? getenv("DEVICE") : "CPU"}|; s|Config init_config = {2,|Config init_config = {3,|' examples/c-example/src/main.c
    mkdir examples/c-example/build && (cd examples/c-example/build && cmake .. && make)
    ```

3. Convert the model. The example sends FP32 tensors, so keep FP32 inputs (about 2 s):

    ```bash
    mkdir -p input output
    cp examples/c-example/artifacts/model.onnx input/
    echo '{ "optimization": { "fp16_compression": false } }' > input/config.json
    (cd input && zip model.zip model.onnx config.json)
    docker run --rm --user $(id -u):$(id -g) -v "$PWD/input":/input -v "$PWD/output":/output \
      oaax-intel-toolchain:latest /input/model.zip /output
    ```

4. Run it on a device:

    === "CPU"

        ```bash
        DEVICE=CPU ./examples/c-example/build/c_example runtime-intel-2.0.0-linux-x86_64/libRuntimeLibrary.so output/model.zip examples/c-example/artifacts/image.jpg
        ```

    === "GPU"

        ```bash
        DEVICE=GPU ./examples/c-example/build/c_example runtime-intel-2.0.0-linux-x86_64/libRuntimeLibrary.so output/model.zip examples/c-example/artifacts/image.jpg
        ```

    === "NPU"

        ```bash
        DEVICE=NPU ./examples/c-example/build/c_example runtime-intel-2.0.0-linux-x86_64/libRuntimeLibrary.so output/model.zip examples/c-example/artifacts/image.jpg
        ```

        Not verified on hardware. Without an NPU the run still succeeds, on the CPU.

You should see (the `Runtime info` line is longer, shortened here):

```
Runtime: OAAX Intel 2.0.0
Runtime info: {"loaded_models":1,...,"available_devices":["CPU","GPU"],"models":[{"id":0,"effective_device":"CPU",...
Output tensor 'bboxes-format:xyxysc;0:face': type 1, shape [12, 6]
  face 1.000 at (0.651, 0.045) - (0.771, 0.297)
  ...
  face 0.999 at (0.654, 0.412) - (0.758, 0.649)
OK: 10/10 requests answered in 8.2 ms (0.8 ms per request)
```

On the GPU the output tensor has shape `[20, 6]` instead of `[12, 6]`: the same 12 faces, then 8 all-zero rows with score `0.000`. The model's row count is dynamic (up to 20) and the GPU returned the full bound, so a host must skip empty rows. **Check `effective_device`**: a failed GPU or NPU setup still exits with `OK` but runs on the CPU. For this tiny model the CPU is faster (8 ms for 10 requests on the CPU, 25 ms on the integrated GPU), and times vary by machine. The run leaves `runtime.log` and a `<hash>.blob` cache file in the current directory.

## Troubleshooting

| Symptom | Cause and fix |
| --- | --- |
| `dtype mismatch for input 'x': model expects f16 but caller sent f32` | The default conversion made every input FP16. Send FP16, or convert with `"fp16_compression": false` |
| `device_type=GPU` or `NPU` but `effective_device` is `CPU` | The device is missing from `available_devices` or its compile failed: `runtime.log` says `Compile on GPU failed ... falling back to CPU`. Install the driver and join the `render` group. In Docker: `--device /dev/dri --group-add <render gid>`. Without access to `/dev/dri`, only `CPU` is listed |
| `Unrecognized device ID` in the log for `NPU` | No NPU is present (or its driver is missing) |
| `GPU` is not the Intel GPU | Another vendor's OpenCL GPU was picked: see the warning above. Install the Intel driver or set `GPU.0` or `GPU.1` |
| One model fails to compile on the GPU (for example `No layout format available for gathernd ... data_type u8`, seen on OpenVINO 2026.1.0) | An operator or type the GPU plugin lacks. The runtime falls back to the CPU; change the model to avoid it for the GPU |
| `runtime_enqueue_input` returns success but no output arrives (`RUNTIME_STATUS_NO_OUTPUT_AVAILABLE`) | The input name or shape does not match the IR, so the request is dropped: look for `Tensor setup failed` in `runtime.log`. Compare with `inputs` in `runtime_get_info()` |
| The GPU returns extra all-zero rows (for example `[20, 6]` instead of `[12, 6]`) | A dynamic output is padded to its upper bound on the GPU. Skip rows whose score is 0 |
| Integer inputs (ids above 65504) give wrong results | They were converted to FP16. Convert with `"fp16_compression": false` |
| `error while loading shared libraries: libgomp.so.1` or `libopenvino.so.2610` | Install `libgomp1`. Keep all the files of the archive in one directory |
| `Failed to extract model archive` | Pass the `.zip` made by the toolchain, not the `.onnx` or the `.xml` |
| `input queue at capacity` | Retrieve outputs faster, or raise `max_queue_size` |
| `CPU does not support AVX2` (`RUNTIME_STATUS_DEVICE_ERROR`) | The CPU is too old. OpenVINO 2026 needs AVX2 |
| `.blob` files appear next to the program | The default `cache_dir` is `.`. Set a path or `""` |
| `10 warnings generated.` on the console | OpenCL compiler messages while compiling for the GPU. Harmless |
