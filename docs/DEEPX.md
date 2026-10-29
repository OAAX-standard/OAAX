> **📦 Implementation details**  
> You can find the implementation of the DEEPX NPUs runtime & toolchain in the [Github repository](https://github.com/OAAX-standard/deepx-acceleration){:target="_blank"}. This page describes its latest release, [v2.0.0](https://github.com/OAAX-standard/deepx-acceleration/releases/tag/v2.0.0){:target="_blank"}.

| At a glance | |
| --- | --- |
| Accelerator | DEEPX NPUs **DX-M1** and **DX-H1** (PCIe / M.2 cards) |
| Engine | DX-RT **3.3.2**, one runtime build per Ubuntu version and CPU architecture |
| Model format | `.dxnn`, produced by the DEEPX compiler DX-COM 2.3.0 |
| Toolchain image | `oaax-deepx-toolchain:2.0.0`, 6.1 GB, x86_64 only, takes a `.zip` bundle |
| Runtime library | `libRuntimeLibrary.so` (about 180 KB), Ubuntu 20.04 / 22.04 / 24.04, x86_64 and aarch64 |
| Runtime input | one `uint8` NHWC tensor, normalization is compiled into the model |
| Not supported | Windows, several models in one process |

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
    DX-COM 2.3.0 reports `Target Hardware: M1`. The repository lists DX-H1 as supported, but this page could not check a DX-H1 conversion. Confirm with DEEPX before relying on it.

## Requirements

### Hardware

- A DX-M1 or DX-H1 card (the DX-M1 is an M.2 Key M module, PCIe Gen3 x4 recommended) and its driver. Needed to **run** only.
- Host CPU x86_64 or aarch64. Conversion needs an **x86_64** host (the image is x86_64 only).

### Software

| | To run a model | To convert a model |
| --- | --- | --- |
| OS | Ubuntu 20.04, 22.04 or 24.04, **the one the runtime was built for** | any Linux with Docker on x86_64 |
| DEEPX software | [DX-RT](https://github.com/DEEPX-AI/dx_rt){:target="_blank"} **3.3.2**, [NPU driver](https://github.com/DEEPX-AI/dx_rt_npu_linux_driver){:target="_blank"}, [firmware](https://github.com/DEEPX-AI/dx_fw){:target="_blank"} | none (DX-COM is in the image) |
| Other | a host application that calls the OAAX API | Docker, 6.1 GB of disk for the image |
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
uvx oaax-manager list        # all builds; * = matches this machine
```

=== "This machine"

    ```bash
    uvx oaax-manager fetch runtime deepx --variant ubuntu$(lsb_release -rs)-dxrt3.3.2
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
docker run --rm --user $(id -u):$(id -g) -v $PWD:/work oaax-deepx-toolchain:2.0.0 /work/model.zip /work/output
```

Output in `./output`: `<onnx name>.dxnn` and `convert.log` (the toolchain writes no `logs.json`). Without `--user` the files belong to root. A 12.7 MB YOLOv8n with 100 calibration images took 3 min 9 s on a 20-core machine.

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
    docker run --rm --user $(id -u):$(id -g) -v $PWD/w:/work -w /work --entrypoint dxcom oaax-deepx-toolchain:2.0.0 \
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

### Behaviour

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

This converts DEEPX's YOLOv8n and runs it on an image with the repository's own test host, which sends a `uint8` NHWC image and decodes the YOLO output.

!!! note "Why not the OAAX C example?"
    The `c-example` model has two inputs and a built-in NMS, and the toolchain rejects it (`DataLoaderError ... expects 2 inputs`). Its host sends float NCHW, which the DEEPX runtime also rejects. The YOLOv8n weights are Ultralytics-derived (AGPL-3.0): use them for evaluation only.

!!! warning "Steps 1 to 6 were run by the page's author, step 7 was not"
    No DEEPX card was available. Steps 1 to 6 ran on Ubuntu 22.04 x86_64; the inference in step 7 was not run, so its output is described, not shown.

1. Install the tools: `sudo apt-get install -y git curl cmake build-essential zip unzip lsb-release`, plus [uv](https://docs.astral.sh/uv/getting-started/installation/){:target="_blank"} and Docker.
2. Fetch the runtime and the toolchain, then load the image:
   ```bash
   uvx oaax-manager fetch runtime deepx --variant ubuntu$(lsb_release -rs)-dxrt3.3.2
   uvx oaax-manager fetch toolchain deepx
   docker load -i oaax-deepx-toolchain-2.0.0.tar
   ```
3. Download the model, its configuration and the calibration images from DEEPX, point the configuration at them and zip:
   ```bash
   mkdir -p calibration_dataset
   curl -fsSL -o yolo8n.onnx https://sdk.deepx.ai/modelzoo/onnx/YOLOV8N-1.onnx
   curl -fsSL -o yolo8n.json https://sdk.deepx.ai/modelzoo/json/YOLOV8N-1.json
   curl -fsSL https://sdk.deepx.ai/dataset/calibration_dataset.tar.gz | tar xz --strip-components=1 -C calibration_dataset
   sed -i 's#"dataset_path": *"[^"]*"#"dataset_path": "./calibration_dataset"#' yolo8n.json
   zip -qr yolo8n.zip yolo8n.onnx yolo8n.json calibration_dataset
   ```
4. Convert (a few minutes; the file name must keep `yolo8` so the test host decodes the output):
   ```bash
   docker run --rm --user $(id -u):$(id -g) -v $PWD:/work oaax-deepx-toolchain:2.0.0 /work/yolo8n.zip /work/out
   ```
   Success ends with `Conversion finished at ...` and `out/yolo8n.dxnn` exists.
5. Prepare the image as raw 640x640 RGB `uint8` (letterboxed, padding 114):
   ```bash
   curl -fsSL -o image.jpg https://raw.githubusercontent.com/OAAX-standard/examples/main/c-example/artifacts/image.jpg
   uv run --no-project --with opencv-python-headless --with numpy python - <<'EOF'
   import cv2, numpy as np
   img = cv2.cvtColor(cv2.imread("image.jpg"), cv2.COLOR_BGR2RGB)
   s = min(640 / img.shape[1], 640 / img.shape[0])
   w, h = int(img.shape[1] * s), int(img.shape[0] * s)
   canvas = np.full((640, 640, 3), 114, np.uint8)
   top, left = (640 - h) // 2, (640 - w) // 2
   canvas[top:top + h, left:left + w] = cv2.resize(img, (w, h))
   canvas.tofile("image_640x640x3.raw")
   EOF
   ```
6. Build the test host against the downloaded runtime (it links without DX-RT installed):
   ```bash
   git clone --branch v2.0.0 --depth 1 https://github.com/OAAX-standard/deepx-acceleration.git
   bash deepx-acceleration/tests/runtime/build-tests.sh --runtime-lib-dir $(echo $PWD/runtime-deepx-*)
   ```
7. **Not run, needs the card, DX-RT 3.3.2, driver and firmware.** Check `dxrt-cli --status` first, then:
   ```bash
   deepx-acceleration/tests/runtime/build/inference_runner --model out/yolo8n.dxnn --input-size 1228800 \
     --input-shape 1,640,640,3 --image image_640x640x3.raw --runs 100 --warmup 10 --csv
   ```
   Expected: log lines on stderr (`[init] runtime initialized`, `[init] model loaded`, the input and output tensor descriptions, then `[yolo8n] N detection(s)` with class, score and box of the first warmup run), and one CSV line on stdout with the average, percentile latencies and throughput. Detections depend on the image: `class=0` is `person` in COCO.

## Troubleshooting

| Symptom | Cause and fix |
| --- | --- |
| `undefined symbol: _ZN4dxrt14GetTaskMaxLoadEv` or `symbol lookup error` when loading or calling the runtime | The installed DX-RT is older than 3.3.2 (reproduced with 2.0.3). Install 3.3.2, run `ldconfig`, check with `ldd` |
| `libdxrt.so: cannot open shared object file` | DX-RT is not installed or not on the library path (it installs to `/usr/local/lib`, run `ldconfig`) |
| `GLIBC_...` / `GLIBCXX_...` not found | Wrong build for the OS: take the variant matching `lsb_release -rs` |
| `[DXRT][Error] Device not found.`, possibly followed by an abort | DX-RT sees no card (seen with `dxrt-cli` and no hardware). Reseat the card and check the driver, firmware and `dxrt-cli --status` |
| `FILE_NOT_FOUND`, `Model file not found: <path>` | Wrong path to the `.dxnn` |
| `INVALID_MODEL`, `Failed to load model: ...` | The file is not a DXNN that this DX-RT accepts (an `.onnx`, truncated, or built by a very different DX-COM) |
| `TENSOR_SHAPE_MISMATCH`, `Input size X does not match model input size Y` | The host sent the wrong tensor, typically float or NCHW. Send `uint8` NHWC of exactly Y bytes |
| `TIMEOUT` from `runtime_enqueue_input` | Too many requests in flight: retrieve outputs, then retry |
| `Multiple models not yet supported` / `Model already loaded` | One model per process: run several processes |
| Conversion: `DataLoaderError ... expects 2 inputs` | Multi-input models are not supported: export a single-input model |
| Conversion: `Input names mismatch` | `inputs` in the JSON does not use the ONNX input names |
| Conversion: `Expected exactly 1 ONNX file, but found 0` | The files must be at the zip root, not in a folder |
| Detections are wrong or empty | The host's preprocessing does not match the `preprocessings` list (RGB order, resize, letterbox) |
