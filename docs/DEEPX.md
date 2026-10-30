> **📦 Implementation details**  
> You can find the implementation of the DEEPX NPUs runtime & toolchain in the [Github repository](https://github.com/OAAX-standard/deepx-acceleration){:target="_blank"}. This page describes its latest release, [v2.0.0](https://github.com/OAAX-standard/deepx-acceleration/releases/tag/v2.0.0){:target="_blank"}.

## At a glance

| | |
| --- | --- |
| Accelerator | DEEPX NPUs **DX-M1** and **DX-H1** (PCIe / M.2 cards) |
| Engine | DX-RT **3.3.2**, one runtime build per Ubuntu version and CPU architecture |
| Model format | `.dxnn`, produced by the DEEPX compiler DX-COM 2.3.0 |
| Toolchain image | `oaax-deepx-toolchain:2.0.0`, 6.1 GB, x86_64 only, takes a `.zip` bundle |
| Runtime library | `libRuntimeLibrary.so` (about 180 KB), Ubuntu 20.04 / 22.04 / 24.04, x86_64 and aarch64 |
| Runtime input | one `uint8` NHWC tensor, normalization is compiled into the model |
| Not supported | Windows, several models in one process |
| Start here | the [end-to-end example](#end-to-end-example): `uvx oaax-manager fetch toolchain deepx` (**6.1 GB** image), then about **5 minutes** of conversion |

## Overview

DEEPX NPUs are edge inference accelerators; DX-H1 is the higher-performance variant of the DX-M1.

- **Toolchain**: converts an ONNX model to DXNN with DX-COM, which is installed in the Docker image (DEEPX permits redistributing it there). It quantizes the model with a calibration image set.
- **Runtime**: implements the OAAX 2.0 interface on top of DX-RT. Inference is asynchronous and results come back in any order, matched by `Tensors.id`.
- **Model constraints** (observed with DX-COM 2.3.0):
    - Single-input models. A two-input model fails with `The dataloader provides a single input, but the model expects 2 inputs`.
    - Static input shapes, declared in the JSON configuration.
    - The compiled model takes `uint8` NHWC input. Only `div` / `normalize` preprocessing is compiled into it; resize, color conversion, crop and transpose are used for calibration only, so the host application must do them.
    - Operators the NPU does not run were placed in a CPU group instead of being rejected (tested with `Erf`, `Einsum`, `CumSum`).
    - Detection models keep their raw output heads: the host decodes boxes and applies NMS.
- **Does not do**: image pre-processing, post-processing, float or layout conversion of the input.

!!! warning "DX-H1"
    DX-COM 2.3.0 reports `Target Hardware: M1`. DX-H1 conversion is not verified yet.

## Requirements

### Hardware

- A DX-M1 or DX-H1 card (the DX-M1 is an M.2 Key M module, PCIe Gen3 x4 recommended) and its driver. Needed to **run** only.
- Host CPU x86_64 or aarch64. Conversion needs an **x86_64** host (the image is x86_64 only).

### Software

| | To run a model | To convert a model |
| --- | --- | --- |
| OS | Ubuntu 20.04, 22.04 or 24.04, **the one the runtime was built for** | any Linux with Docker on x86_64 |
| DEEPX software | [DX-RT](https://github.com/DEEPX-AI/dx_rt){:target="_blank"} **3.3.2**, [NPU driver](https://github.com/DEEPX-AI/dx_rt_npu_linux_driver){:target="_blank"}, [firmware](https://github.com/DEEPX-AI/dx_fw){:target="_blank"} | none (DX-COM is in the image) |
| Other | a host application that calls the OAAX API; on a minimal image `libgomp1` | Docker, [uv](https://docs.astral.sh/uv/getting-started/installation/){:target="_blank"} (for `oaax-manager`), 6.1 GB of disk for the image |
| Network | no | **no**: converting works with `--network none` (tested); only downloading the image needs it |

!!! warning "The DX-RT version must match exactly"
    The runtime links DX-RT dynamically and was built against 3.3.2. An older DX-RT fails to load it (see [Troubleshooting](#troubleshooting)).

Check what is installed:

```bash
dxrt-cli --status                        # first line: DXRT v<version>; also lists the driver, firmware and device
ldd libRuntimeLibrary.so | grep dxrt     # which libdxrt.so the runtime resolves
```

DX-RT is built from source (`./install.sh --all && ./build.sh` in the DX-RT repository, as the runtime's Docker build does); follow the DEEPX instructions for the driver and firmware. Windows is not supported yet: the Windows build job is disabled because the DX-RT Windows SDK repository is not reachable from CI.

## Get the runtime and toolchain

With [`oaax-manager`](https://pypi.org/project/oaax-manager/){:target="_blank"}, which verifies the checksums:

```bash
uvx oaax-manager list        # all builds; * = matches this CPU architecture (all three Ubuntu variants get a star: pick yours)
```

=== "This machine"

    ```bash
    . /etc/os-release       # sets VERSION_ID, e.g. 22.04
    uvx oaax-manager fetch runtime deepx --variant ubuntu$VERSION_ID-dxrt3.3.2    # --variant is required
    uvx oaax-manager fetch toolchain deepx     # 6.1 GB, saves oaax-deepx-toolchain-2.0.0.tar
    ```

=== "Another CPU (e.g. an aarch64 board)"

    ```bash
    uvx oaax-manager fetch runtime deepx --arch aarch64 --variant ubuntu22.04-dxrt3.3.2
    ```

The variants are `ubuntu20.04-dxrt3.3.2`, `ubuntu22.04-dxrt3.3.2` and `ubuntu24.04-dxrt3.3.2`. The runtime lands in `runtime-deepx-2.0.0-linux-<arch>-<variant>/libRuntimeLibrary.so`. The SHA-256 of every file is in the [release notes](https://github.com/OAAX-standard/deepx-acceleration/releases/tag/v2.0.0){:target="_blank"}.

### Download links and compatibility matrix

Or download by hand. Runtime libraries are packaged as `library-dxrt-v<DX-RT version>.tar.gz` and contain `libRuntimeLibrary.so`. Pick the build for your Ubuntu version and CPU architecture, then install the DX-RT version in its row. The SHA-256 of each file is in the [release notes](https://github.com/OAAX-standard/deepx-acceleration/releases/tag/v2.0.0){:target="_blank"}.

#### OAAX 2.0

| OAAX version | OS      | Version | CPU architecture | DX-RT | Runtime library | Conversion toolchain |
| ------------ | ------- | ------- | ---------------- | ----- | --------------- | -------------------- |
| 2.0.0        | Ubuntu  | 20.04   | x86_64           | 3.3.2 | [Download](https://nbg1.your-objectstorage.com/oaax/runtimes/2.0.0/DEEPX/x86_64/Ubuntu/20.04/library-dxrt-v3.3.2.tar.gz) | [Download](https://nbg1.your-objectstorage.com/oaax/conversion-toolchain/2.0.0/DEEPX/oaax-deepx-toolchain.tar) (6.1 GB) |
| 2.0.0        | Ubuntu  | 20.04   | aarch64          | 3.3.2 | [Download](https://nbg1.your-objectstorage.com/oaax/runtimes/2.0.0/DEEPX/aarch64/Ubuntu/20.04/library-dxrt-v3.3.2.tar.gz) | ⬆️ |
| 2.0.0        | Ubuntu  | 22.04   | x86_64           | 3.3.2 | [Download](https://nbg1.your-objectstorage.com/oaax/runtimes/2.0.0/DEEPX/x86_64/Ubuntu/22.04/library-dxrt-v3.3.2.tar.gz) | ⬆️ |
| 2.0.0        | Ubuntu  | 22.04   | aarch64          | 3.3.2 | [Download](https://nbg1.your-objectstorage.com/oaax/runtimes/2.0.0/DEEPX/aarch64/Ubuntu/22.04/library-dxrt-v3.3.2.tar.gz) | ⬆️ |
| 2.0.0        | Ubuntu  | 24.04   | x86_64           | 3.3.2 | [Download](https://nbg1.your-objectstorage.com/oaax/runtimes/2.0.0/DEEPX/x86_64/Ubuntu/24.04/library-dxrt-v3.3.2.tar.gz) | ⬆️ |
| 2.0.0        | Ubuntu  | 24.04   | aarch64          | 3.3.2 | [Download](https://nbg1.your-objectstorage.com/oaax/runtimes/2.0.0/DEEPX/aarch64/Ubuntu/24.04/library-dxrt-v3.3.2.tar.gz) | ⬆️ |

## Conversion toolchain

### Run it

The input is a `.zip` with, **at its root**, exactly one `.onnx`, exactly one `.json` configuration and optionally a calibration image folder.

```bash
docker load -i oaax-deepx-toolchain-2.0.0.tar     # loads oaax-deepx-toolchain:2.0.0 and :latest
docker run --rm --user $(id -u):$(id -g) -v "$PWD:/work" oaax-deepx-toolchain:2.0.0 /work/model.zip /work/output
```

Output in `./output`: `<onnx name>.dxnn` and `convert.log` (the toolchain writes no `logs.json`). Without `--user` the files belong to root. The official YOLOX-s with 100 calibration images took 4 min 42 s on a 20-core machine (see the example below).

### Options

The entrypoint takes only the two paths and reads no environment variable. Everything else goes in the JSON file:

| JSON key | Meaning |
| --- | --- |
| `inputs` | **Required.** `{"<input name>": [N, C, H, W]}`; must match the ONNX input names and shapes, else `Input names mismatch` / `Input shape mismatch` |
| `default_loader.dataset_path` | Calibration image folder, relative to the zip root (`./calibration_dataset`) |
| `default_loader.file_extensions` | Image extensions to use, e.g. `["jpeg", "jpg", "png"]`; none found gives `DataNotFoundError` |
| `default_loader.preprocessings` | Ordered list applied to each calibration image, below |
| `calibration_num` | Number of calibration images used (100 in the DEEPX model zoo); more is slower |
| `calibration_method` | `"ema"` in all DEEPX examples. Other values are not validated by 2.3.0 (`"foo"` was accepted), see the DEEPX documentation |
| `train_batchsize`, `num_samples` | Present in the model-zoo YOLO configs, see the DEEPX documentation |

Without `default_loader` the compiler warns `No Data Loader is specified. Use Dummy Dataloader` and still compiles, but the quantization is not calibrated on real data and accuracy may drop.

Preprocessing steps: `resize` (`width`+`height`, or `mode`, `size`, `pad_location`, `pad_value`, `interpolation`), `convertColor` (`form`, e.g. `BGR2RGB`), `centercrop` (`width`, `height`), `div` (`x`), `normalize` (`mean`, `std`), `transpose` (`axis`), `expandDim` (`axis`). An unknown step fails with `Preprocessing transform '<name>' not found` (`sub` does not exist). The log line `Added nodes: Div(...)` lists what was compiled into the model, `Skipped nodes` what the host must do. DEEPX's [model zoo](https://developer.deepx.ai/modelzoo/){:target="_blank"} has ready-made configurations.

??? note "Advanced: DX-COM flags (bypass the entrypoint)"
    The entrypoint does not forward flags. To use them, run `dxcom` directly (tested with `--opt_level 0 --gen_log`):

    ```bash
    docker run --rm --user $(id -u):$(id -g) -v "$PWD/w:/work" -w /work --entrypoint dxcom oaax-deepx-toolchain:2.0.0 \
      -m model.onnx -c model.json -o out --opt_level 0 --gen_log
    ```

    | Flag | Effect |
    | --- | --- |
    | `--opt_level {0,1}` | default 1: faster NPU model, longer compile; 0: the opposite |
    | `--aggressive_partitioning` | run as many operations as possible on the NPU |
    | `--compile_input_nodes`, `--compile_output_nodes` | comma-separated node names to compile from / up to |
    | `--float64_calibration` | float64 calibration, deterministic across CPUs |
    | `--gen_log` | also write `compiler.log` and `error.log` to the output folder |

### Errors

| Message | Cause |
| --- | --- |
| `Usage: ... <zip file> <output dir>` | not exactly two arguments |
| `Expected exactly 1 ONNX file, but found N` / `... JSON file ...` | wrong count, or the files are in a sub-folder of the zip, or the zip path is wrong |
| `Error: dxcom conversion failed.` | followed by DX-COM's own error, e.g. `ConfigFileError: You Must define input shapes`, `ConfigInputError`, `DataNotFoundError`, `DataPreprocessingError`, `DataLoaderError` |

Limits: the toolchain writes `convert.log` instead of `logs.json`, invalid config keys and `calibration_method` values are accepted silently, only a zip bundle is accepted, and the image is large (6.1 GB).

## Runtime library

The runtime implements the [OAAX 2.0 interface](https://github.com/OAAX-standard/OAAX/blob/main/include/interface.h){:target="_blank"} (`runtime_get_name()` returns `DEEPX`, `runtime_get_version()` `2.0.0`).

### Configuration

`runtime_init` reads **no** `Config` key: pass `length = 0` (other keys are only logged at debug level). Settings are environment variables:

| Variable | Default | Effect |
| --- | --- | --- |
| `DEEPX_RUNTIME_LOG_FILE` | `runtime.log` | log file (truncated at each `runtime_init`); logs also go to stderr |
| `SPDLOG_LEVEL` | `info` | log verbosity (`trace`, `debug`, `info`, `warn`, `error`, `critical`, `off`) |
| `DXRT_DYNAMIC_CPU_THREAD` | `ON` (set by the runtime unless already set) | DX-RT thread pool for the model's CPU parts; `OFF` is respected |

`ModelConfig`: only `file_path` (the `.dxnn` file) is used. `model_data`, `model_size` and `config` are ignored, so a model cannot be loaded from memory.

### Behavior

- **One model per process**: `runtime_load_models` needs `num_models == 1` and refuses a second model.
- **Input**: only the first tensor is read. Its `data_size` must equal the model's input size; name, type and shape are not checked, so send `uint8` NHWC.
- **Output**: one tensor per model output, in the type and shape DX-RT reports (`UNDEFINED` for any type other than the 8 to 64-bit integers and float32). The caller frees it.
- **Concurrency**: enqueue and retrieve can run on different threads. Results are unordered, so match them by `Tensors.id`. `timeout_ms`: `0` polls, positive waits, negative waits forever.
- **Backpressure**: the output pool holds DX-RT's in-flight limit times the number of devices. When full, `runtime_enqueue_input` returns `RUNTIME_STATUS_TIMEOUT` at once: retry or drop the frame.
- `runtime_get_info()` returns JSON: `loaded_models`, `requests_in_flight`, `output_queue`, `pool_in_use`, `pool_size`, `pool_capacity`. `runtime_cleanup()` waits for in-flight requests.

| Status | When |
| --- | --- |
| `NOT_INITIALIZED` / `ALREADY_INITIALIZED` | call before `runtime_init` / second `runtime_init` |
| `INVALID_ARGUMENT` | `num_models != 1`, a model is already loaded, null `file_path` |
| `FILE_NOT_FOUND` | the `.dxnn` file cannot be opened |
| `INVALID_MODEL` | DX-RT rejects the file (`Failed to load model: ...`) |
| `MODEL_NOT_LOADED`, `INVALID_MODEL_ID` | enqueue with no model / a `model_id` other than 0 |
| `INVALID_TENSOR` | no input tensor |
| `TENSOR_SHAPE_MISMATCH` | input size differs from the model's |
| `TIMEOUT` | output pool exhausted (backpressure) |
| `INFERENCE_ERROR` | DX-RT `RunAsync` failed; the caller keeps the input |
| `NO_OUTPUT_AVAILABLE` | retrieve timed out |
| `OUT_OF_MEMORY` | the output tensors could not be built |

## End-to-end example

This converts the official **YOLOX-s** detector and runs it with the [YOLOX example](https://github.com/OAAX-standard/examples/tree/main/yolox-example){:target="_blank"}, one small C host that runs on every OAAX runtime; for DEEPX only two flags differ.

!!! note "What step 6 needs"
    A DEEPX card, DX-RT 3.3.2, the driver and the firmware.

!!! note "Before you start"
    Step 3 downloads the **6.1 GB** toolchain image and step 4 takes about **5 minutes** (4 min 42 s here, including the calibration download).

1. Install the tools (Ubuntu), plus [uv](https://docs.astral.sh/uv/getting-started/installation/){:target="_blank"} and [Docker](https://docs.docker.com/engine/install/){:target="_blank"}:
   ```bash
   sudo apt-get install -y git curl cmake build-essential zip unzip libgomp1
   ```
2. Get the example, build the host and download the model (the sha256 is checked):
   ```bash
   git clone https://github.com/OAAX-standard/examples.git
   cd examples/yolox-example
   cmake -S . -B build && cmake --build build
   bash fetch_model.sh
   ```
3. Fetch the runtime and the toolchain, load the image:
   ```bash
   . /etc/os-release
   uvx oaax-manager fetch runtime deepx --variant ubuntu$VERSION_ID-dxrt3.3.2
   uvx oaax-manager fetch toolchain deepx
   docker load -i oaax-deepx-toolchain-2.0.0.tar
   ```
4. Convert. The bundle holds the model, [`configs/deepx.json`](https://github.com/OAAX-standard/examples/blob/main/yolox-example/configs/deepx.json){:target="_blank"} and 100 calibration images from DEEPX:
   ```bash
   mkdir -p deepx/calibration_dataset
   curl -fsSL https://sdk.deepx.ai/dataset/calibration_dataset.tar.gz | tar xz --strip-components=1 -C deepx/calibration_dataset
   cp models/yolox_s.onnx deepx/ && cp configs/deepx.json deepx/yolox_s.json
   (cd deepx && zip -qr yolox_s.zip yolox_s.onnx yolox_s.json calibration_dataset)
   docker run --rm --user "$(id -u):$(id -g)" -v "$PWD/deepx:/work" oaax-deepx-toolchain:2.0.0 /work/yolox_s.zip /work/out
   ```
   It ends with `Conversion finished at ...` and `deepx/out/yolox_s.dxnn` exists.
5. Check that the host and the runtime are in place: `ls build/yolox_host runtime-deepx-*/libRuntimeLibrary.so`.
6. **Run on the card.** Check `dxrt-cli --status` (DX-RT 3.3.2), then run the host with the DEEPX flags (`--layout nhwc --dtype uint8`, the converted model takes raw uint8 NHWC pixels):

=== "Same machine"

    ```bash
    ./build/yolox_host runtime-deepx-*/libRuntimeLibrary.so deepx/out/yolox_s.dxnn artifacts/dog-bicycle.jpg \
      --layout nhwc --dtype uint8
    ```

=== "Convert on x86_64, run on an aarch64 board"

    Fetch the runtime on the board (`uvx oaax-manager fetch runtime deepx --variant ubuntu$VERSION_ID-dxrt3.3.2`), build the host there (steps 1 and 2), copy `deepx/out/yolox_s.dxnn` to it and run the same command.

The host prints the runtime name (`DEEPX`), the model's output tensor, the detections and `OK: 10 requests ...`, and writes `detections.png`. Output on a DX-M1 (DX-RT 3.3.2, Ubuntu 24.04):

```
Runtime: DEEPX 2.0.0
Input: 'images' uint8 NHWC, 1280x854 image letterboxed to 640
Output tensor 'output': type 1, shape [1, 8400, 85]
Detections (score >= 0.30, NMS IoU 0.45): 2
  bicycle        0.949  ( 613,  210) - (1228,  804)
  dog            0.927  ( 212,  247) - ( 521,  587)
Wrote detections.png
OK: 10 requests, first 34.6 ms (warm-up), then 24.3 ms on average (min 22.7 ms)
```

The same two objects as on the CPU runtime; boxes are within 4 px and scores within 0.01 because the model is quantized to INT8. Times depend on the host.

## Troubleshooting

| Symptom | Cause and fix |
| --- | --- |
| `undefined symbol: _ZN4dxrt14GetTaskMaxLoadEv` or `symbol lookup error` when loading or calling the runtime | The installed DX-RT is older than 3.3.2 (reproduced with 2.0.3). Install 3.3.2, run `ldconfig`, check with `ldd` |
| `libdxrt.so: cannot open shared object file` | DX-RT is not installed or not on the library path (it installs to `/usr/local/lib`, run `ldconfig`) |
| `GLIBC_...` / `GLIBCXX_...` not found | Wrong build for the OS: take the variant matching your Ubuntu (`. /etc/os-release; echo $VERSION_ID`) |
| `Device not found` from `dxrt-cli` (also `[DXRT][Error] Device not found.`, possibly followed by an abort), while `lspci` lists the card | The driver is not loaded: `lsmod | grep dx` is empty and `/dev/dxrt0` is missing. The driver is built for one kernel, so a kernel update removes it. In `dx_rt_npu_linux_driver/modules` run `./build.sh` (needs `linux-headers-$(uname -r)`), `sudo ./build.sh -c uninstall`, `sudo ./build.sh -c install --reload`, then check `dxrt-cli --status`. If `lspci` does not list the card either, reseat it |
| `FILE_NOT_FOUND`, `Model file not found: <path>` | Wrong path to the `.dxnn` |
| `INVALID_MODEL`, `Failed to load model: ...` | The file is not a DXNN that this DX-RT accepts (an `.onnx`, truncated, or built by a very different DX-COM) |
| `TENSOR_SHAPE_MISMATCH`, `Input size X does not match model input size Y` | The host sent the wrong tensor, typically float or NCHW. Send `uint8` NHWC of exactly Y bytes |
| `TIMEOUT` from `runtime_enqueue_input` | Too many requests in flight: retrieve outputs, then retry |
| `Multiple models not yet supported` / `Model already loaded` | One model per process: run several processes |
| Conversion: `DataLoaderError ... expects 2 inputs` | Multi-input models are not supported: export a single-input model |
| Conversion: `Input names mismatch` | `inputs` in the JSON does not use the ONNX input names |
| Conversion: `Expected exactly 1 ONNX file, but found 0` | The files must be at the zip root, not in a folder |
| Detections are wrong or empty | The host's preprocessing does not match the `preprocessings` list (RGB order, resize, letterbox) |
