> **📦 Implementation details**  
> You can find the implementation of the Hailo runtime & toolchain in the [Github repository](https://github.com/OAAX-standard/hailo-acceleration){:target="_blank"}. This page describes the [latest release (v2.0.0)](https://github.com/OAAX-standard/hailo-acceleration/releases/tag/v2.0.0){:target="_blank"}.

## At a glance

| | |
| --- | --- |
| **Start here** | Check your HailoRT version ([how](#software)), then follow the [end-to-end example](#end-to-end-example). |
| **NPU class** | Dedicated AI accelerator on a PCIe / M.2 module |
| **Chips** | Hailo-8 |
| **HailoRT versions** | 4.17.0, 4.18.0, 4.19.0, 4.20.0: one runtime build per version |
| **Engine** | ONNX Runtime with the Hailo execution provider, on top of HailoRT |
| **Model format** | ONNX in, `<model>.hailo8.onnx` out (the compiled model is wrapped in an ONNX file) |
| **Runtime library** | `libRuntimeLibrary.so` (Ubuntu, x86_64 and aarch64) |
| **Toolchain image** | `onnx-to-hailo:latest` (Docker, x86_64 hosts only, 1.2 GB, needs Hailo's SDK files) |
| **How far it is verified** | Run on a Hailo-8 with HailoRT 4.20.0. The 4.17 to 4.19 runtimes are only compiled and symbol-checked by CI. Hailo-8L is not verified ([issue #14](https://github.com/OAAX-standard/hailo-acceleration/issues/14)). |

## Overview

- **Toolchain**: unpacks a zip (ONNX model, an options JSON, optional calibration images) and runs the Hailo SDK steps *parse*, *optimize* (INT8 quantization with the calibration images) and *compile* for the `hailo8` architecture. It writes `<model>.hailo8.onnx`: a graph whose `HailoOp` node embeds the compiled model, followed by the parts of the original graph the chip cannot run.
- **Runtime**: loads that file with ONNX Runtime; the Hailo execution provider runs the `HailoOp` node on the chip through HailoRT and everything else on the host CPU. Inputs and outputs keep the types and layout of the converted graph (float32 NCHW in and out for YOLOX and YOLOv8).
- **One runtime build per HailoRT version**: each archive bundles its own `libhailort`, which must match the HailoRT driver installed on the host.
- **Model constraints**: the Hailo SDK decides which operators convert; the repository does not list them. It documents YOLOv8n, YOLOv8s and YOLOX-s as converting, and YOLO11n and YOLO26n as failing (attention layers, SDK 3.25.0). The model needs one image input with a fixed size.
- **Not done**: no Hailo-8L target, no model script or optimization level, no batch or device selection, no accuracy check, no decoding of detections (the host does it).

!!! warning "Known gaps"
    A wrong input name, an unconverted model and a failed conversion do not fail loudly (see [Troubleshooting](#troubleshooting) and issues [#8](https://github.com/OAAX-standard/hailo-acceleration/issues/8) to [#17](https://github.com/OAAX-standard/hailo-acceleration/issues/17)). The toolchain compiles for `hailo8` only.

## Requirements

### Hardware

- A **Hailo-8** module (PCIe or M.2) in an x86_64 or aarch64 host.
- Conversion needs an **x86_64** Linux machine (the image and Hailo's compiler are x86_64 only). Copy the converted model to an aarch64 device afterwards.

### Software

| Needed to | Requirement |
| --- | --- |
| **Run** | Ubuntu 20.04 or higher (the libraries need glibc 2.29 and libstdc++ from GCC 9; Ubuntu 22.04 was used for this page) |
| | `libatomic1` and `libgomp1`: `sudo apt-get install libatomic1 libgomp1` |
| | Hailo's PCIe driver (`hailort-pcie-driver`) installed, with `/dev/hailo*` present |
| | A runtime build whose HailoRT version **equals the installed driver version** (below) |
| **Convert** | Docker, and internet access (the container installs the SDK's Python dependencies at every start) |
| | From Hailo's [Developer Zone](https://hailo.ai/developer-zone/){:target="_blank"} (login required, not redistributable): `hailo_dataflow_compiler-3.<minor>.0-py3-none-linux_x86_64.whl`, `hailort-4.<minor>.0-cp310-cp310-linux_x86_64.whl` and the matching `hailort_4.<minor>.0_amd64.deb` (it provides `libhailort`, which the wheel needs; an empty or placeholder file is not enough), **one version of each** in one folder |

The image holds Ubuntu 22.04 and Python 3.10 (hence `cp310`). The example below was converted with Dataflow Compiler 3.29.0 and the HailoRT 4.19.0 wheel; which compiler goes with which HailoRT is not documented ([issue #15](https://github.com/OAAX-standard/hailo-acceleration/issues/15)). A model converted that way runs on the 4.20.0 runtime.

**Pick the runtime variant**: it is the version of the installed driver.

```bash
hailortcli --version                 # HailoRT-CLI version 4.20.0
dpkg -l | grep -i hailo              # hailort and hailort-pcie-driver, same version
modinfo hailo_pci | grep ^version    # driver version
hailortcli scan                      # lists the devices
```

## Get the runtime and toolchain

[oaax-manager](https://pypi.org/project/oaax-manager/) (0.1.1 or newer, which fixed the Hailo checksums; check with `uvx oaax-manager --version`) downloads and verifies both. Install [uv](https://docs.astral.sh/uv/getting-started/installation/) first.

```bash
uvx oaax-manager list                                                         # Hailo rows: hailort4.17.0 ... hailort4.20.0
uvx oaax-manager fetch runtime hailo --variant hailort4.20.0                  # -> runtime-hailo-2.0.0-linux-x86_64-hailort4.20.0/
uvx oaax-manager fetch runtime hailo --variant hailort4.20.0 --arch aarch64   # for an aarch64 device
uvx oaax-manager fetch toolchain hailo                                        # -> oaax-hailo-toolchain-2.0.0.tar
```

`--variant` is required (several builds exist). The runtime folder holds `libRuntimeLibrary.so`, `libonnxruntime_providers_hailo.so`, `libonnxruntime_providers_shared.so`, `libhailort.so.<version>` and `oaax_runtime.h`. **Keep them together**: the companion libraries are found next to `libRuntimeLibrary.so`.

### Download links and compatibility matrix

Runtime libraries are packaged as `library-<HailoRT version>.tar.gz` (same files as above). SHA-256 sums are in the [release notes](https://github.com/OAAX-standard/hailo-acceleration/releases/tag/v2.0.0){:target="_blank"}; sizes are in brackets.

#### OAAX 2.0

Conversion toolchain (any platform with Docker): [Download](https://nbg1.your-objectstorage.com/oaax/conversion-toolchain/2.0.0/HAILO/oaax-hailo-toolchain.tar) (1.2 GB)

| OS | CPU architecture | HailoRT | Runtime library |
| -- | ---------------- | ------- | --------------- |
| Ubuntu | x86_64 | 4.17.0 | [Download](https://nbg1.your-objectstorage.com/oaax/runtimes/2.0.0/HAILO/x86_64/Ubuntu/library-4.17.0.tar.gz) (8 MB) |
| Ubuntu | x86_64 | 4.18.0 | [Download](https://nbg1.your-objectstorage.com/oaax/runtimes/2.0.0/HAILO/x86_64/Ubuntu/library-4.18.0.tar.gz) (8 MB) |
| Ubuntu | x86_64 | 4.19.0 | [Download](https://nbg1.your-objectstorage.com/oaax/runtimes/2.0.0/HAILO/x86_64/Ubuntu/library-4.19.0.tar.gz) (8 MB) |
| Ubuntu | x86_64 | 4.20.0 | [Download](https://nbg1.your-objectstorage.com/oaax/runtimes/2.0.0/HAILO/x86_64/Ubuntu/library-4.20.0.tar.gz) (8 MB) |
| Ubuntu | aarch64 | 4.17.0 | [Download](https://nbg1.your-objectstorage.com/oaax/runtimes/2.0.0/HAILO/aarch64/Ubuntu/library-4.17.0.tar.gz) (7 MB) |
| Ubuntu | aarch64 | 4.18.0 | [Download](https://nbg1.your-objectstorage.com/oaax/runtimes/2.0.0/HAILO/aarch64/Ubuntu/library-4.18.0.tar.gz) (7 MB) |
| Ubuntu | aarch64 | 4.19.0 | [Download](https://nbg1.your-objectstorage.com/oaax/runtimes/2.0.0/HAILO/aarch64/Ubuntu/library-4.19.0.tar.gz) (8 MB) |
| Ubuntu | aarch64 | 4.20.0 | [Download](https://nbg1.your-objectstorage.com/oaax/runtimes/2.0.0/HAILO/aarch64/Ubuntu/library-4.20.0.tar.gz) (8 MB) |

## Conversion toolchain

### Run it

1. Put the three SDK files in `hailo-deps/` and make a zip with the model and an options JSON (below). Without calibration images the toolchain uses the 100 COCO images it ships with (the SDK used 64 of them in the YOLOX run).
2. Load the image. Its name and tag come from the tar: **`onnx-to-hailo:latest`** (no versioned tag, [issue #16](https://github.com/OAAX-standard/hailo-acceleration/issues/16)).
3. Run it. **Do not pass `--user`**: the container installs the SDK as root and fails otherwise ([issue #9](https://github.com/OAAX-standard/hailo-acceleration/issues/9)). Pass `-e PIP_FIND_LINKS=https://storage.googleapis.com/jax-releases/jax_releases.html` with Dataflow Compiler 3.29: it requires `jaxlib==0.4.13`, which is gone from PyPI.

```bash
docker load -i oaax-hailo-toolchain-2.0.0.tar
docker run --rm -e PIP_FIND_LINKS=https://storage.googleapis.com/jax-releases/jax_releases.html \
  -v "$PWD/hailo-deps:/app/hailo-deps" -v "$PWD/input:/app/input:ro" -v "$PWD/output:/app/output" \
  onnx-to-hailo:latest /app/input/model.zip /app/output
sudo chown -R "$(id -u):$(id -g)" output     # the container wrote as root
```

The entry point takes exactly two arguments (zip path, output folder, both inside the container). Every start re-installs the SDK; the conversion itself took about 14 minutes for YOLOX-s.

### Zip contents

| File | Rule |
| --- | --- |
| `*.onnx` | Exactly one, at the top level. Its name decides the output name. |
| `*.json` | Exactly one, at the top level: the options below. |
| `*.jpg`, `*.jpeg` | Optional calibration images, anywhere in the zip. Resized (stretched) to `width` x `height`. |

### Options JSON

| Key | Type | Default | Meaning |
| --- | --- | --- | --- |
| `height`, `width` | int | required | Model input size; calibration images are resized to it. |
| `channels` | int | required | `1` = grayscale calibration images, anything else = RGB. |
| `nchw` | bool | required | Layout of the calibration array. Keep `false`: Hailo expects NHWC data whatever the model's layout. |
| `means`, `stds` | list of numbers | required | Calibration pixels become `(pixel - mean) / std` (pixels are 0 to 255). **Give the runtime the same normalization.** One value or one per channel. |
| `start_node_names` | list of strings | `[]` | Where the SDK starts parsing; empty = the model's inputs. |
| `end_node_names` | list of strings | `[]` | Where the SDK stops parsing; the nodes after stay plain ONNX on the host. Empty = auto-detect the YOLOv8 head (finds only 6 of the 9 head nodes of YOLOX, so list them); other models: the whole graph. |

Other keys are ignored. The YOLOX-s file of the example (`configs/hailo.json` in the examples repository):

```json
{"start_node_names": [],
 "end_node_names": ["Sigmoid_264", "Conv_261", "Sigmoid_263", "Sigmoid_285", "Conv_282", "Sigmoid_284", "Sigmoid_306", "Conv_303", "Sigmoid_305"],
 "height": 640, "width": 640, "channels": 3, "nchw": false, "means": [0.0, 0.0, 0.0], "stds": [1.0, 1.0, 1.0]}
```

### Outputs, logs and errors

The output folder gets `<model>.hailo8.onnx` and `logs.json` (and possibly a `tmp/` folder: the unzipped input). `logs.json` is a list of `{"Message": ..., "Data": {...}}`:

| Message | `Data` |
| --- | --- |
| `Converting ONNX to Hailo-ONNX` | `Input Path` |
| `Starting conversion` | `Target Hardware` (`hailo8`); stages `Translation`, `Optimizing Model`, `Compiling Model` set to `Done` as they finish |
| `Auto-detected end nodes` | `nodes`: the cut found for YOLOv8 models |
| `Successful Conversion` (last) | `MIME type` (`application/x-onnx; device=hailo-8`), `Output Path` |
| `Conversion Failed` (last) | `Human Error` (always the same generic text), `Technical Error` (the real reason) |

!!! warning "The exit code is 0 even when the conversion fails"
    Check that the last message is `Successful Conversion` ([issue #8](https://github.com/OAAX-standard/hailo-acceleration/issues/8)). Only the failures before the conversion below give a non-zero exit code.

| Message | Meaning and fix |
| --- | --- |
| `dpkg: error: cannot access archive '../hailo-deps/hailort_4.*.0_amd64.deb'` (exit 2) | `hailo-deps` not mounted, or the file is missing or misnamed. |
| `dpkg: error: requested operation requires superuser privilege` (exit 2) | You passed `--user`. Remove it. |
| `Usage: /app/scripts/convert.sh <onnx-path> <output-directory>` (exit 1) | Wrong number of arguments. |
| `Could not find a version that satisfies the requirement jaxlib==0.4.13` | Add `PIP_FIND_LINKS` (above). |
| Technical Error `No module named 'hailo_sdk_client'` | The SDK wheels were not installed; read the container's log above. |
| Technical Error `The HailoRT Python API (hailo_platform package) must be installed` | After a **14-minute compile**: the HailoRT wheel is there but its `libhailort` (from the `.deb`) is not. Install the real `.deb` of the same version. |
| Technical Error `File is not a zip file`, `No ONNX file found in the zip`, `More than one ONNX file found in the zip` (same for JSON) | Fix the zip. |
| Technical Error `'means'` (or another key) | A required options key is missing. |
| Technical Error from the SDK parser (may recommend other end nodes) | An operator is not supported. Use the recommended `end_node_names` / `start_node_names`, or another model. |

The compiler warns that with fewer than 1024 calibration images and no GPU it lowers the optimization level; this is why INT8 boxes can differ by about 10 pixels from the CPU run. Give it more images if that matters.

## Runtime library

The library implements the OAAX [runtime interface](Specification.md#oaax-runtime-specification). Unknown config keys are ignored.

### `runtime_init` keys

| Key | Values | Default | Meaning |
| --- | --- | --- | --- |
| `log_level` | `0` debug, `1` info, `2` warning, `3` error | `1` | Minimum level written to the log file. Out of range = `1`; a non-numeric value counts as `0`. |
| `log_file` | path | `runtime.log` | Base name: the files are `<log_file>.0`, `.1`, ... (a new one every 4 MiB, appended to). The console always gets INFO and above. |
| `n_threads` | `1` to `16` | `4` | ONNX Runtime threads in total, split evenly over all replicas of all models (at least 1 each). Above 16 = 16; `0`, negative or non-numeric = ignored (`perf_mode` applies). |
| `perf_mode` | `eco`, `power` | unset | Only used when `n_threads` is not set: 40% (`eco`) or 90% (`power`) of the online cores, at least 1. Another value = 4. |
| `n_replicas` | `1` or more | `1` | ONNX Runtime sessions, each with its own worker thread, per model. Below 1 = 1. |

The threads only run the ONNX nodes that stay on the CPU (for YOLOX: slicing in front, reshapes behind); the chip works inside HailoRT. There is no device-selection key. HailoRT writes its own `hailort.log` in the working directory.

### Models and inputs

- `ModelConfig`: give `file_path` (or `model_data` + `model_size`). `ModelConfig.config` is unused. At most **8 models**; a model's id is its index in the array. Models load once per `runtime_init`.
- **Inputs are bound by tensor name**, in any order. The name must be the one in the *converted* model: the SDK renames the input to `input0`, so read it from the `.hailo8.onnx` file (`onnx.load(path).graph.input`), not from your original model ([issue #10](https://github.com/OAAX-standard/hailo-acceleration/issues/10)).
- Each model has an input queue and the runtime one output queue, 100 items each. When full, the **oldest** item is dropped and `runtime_enqueue_input` still succeeds: keep retrieving. With several replicas outputs can come back in any order: match them by `Tensors.id`.
- Enqueueing and retrieving can be done from different threads. To switch models, call `runtime_cleanup` and `runtime_init` again. The library exports only the OAAX API (checked in CI), so a host can unload it and load another copy.

### Status codes and errors

| Call | Status | When |
| --- | --- | --- |
| `runtime_init` | `ALREADY_INITIALIZED` (3) | Called twice without `runtime_cleanup`. |
| `runtime_load_models` | `NOT_INITIALIZED` (2), `ALREADY_INITIALIZED` (3), `INVALID_ARGUMENT` (5) | No init; models already loaded; fewer than 1 or more than 8 models. |
| | `ERROR` (1) | Any model failure (missing file, unsupported model, chip unreachable, version mismatch). `runtime_get_error()` says `Failed to create ORT session for replica N`; **the reason is in the log** (`ORT error: ...`). |
| `runtime_enqueue_input` | `NOT_INITIALIZED` (2), `MODEL_NOT_LOADED` (4), `INVALID_MODEL_ID` (18), `INVALID_ARGUMENT` (5) | Wrong call order, bad id, `NULL` tensors. |
| `runtime_retrieve_output` | `NO_OUTPUT_AVAILABLE` (12) | Nothing within the timeout (`0` = do not wait, negative = wait forever). |

!!! warning "A bad input is not reported to the caller"
    A wrong tensor name, type or shape is **accepted** by `runtime_enqueue_input`. The request fails in the worker, is logged (`Inference run failed` and the ONNX Runtime reason) and no output arrives: the host sees `NO_OUTPUT_AVAILABLE` ([issue #12](https://github.com/OAAX-standard/hailo-acceleration/issues/12)).

`runtime_get_info()` returns `{"loaded_models":N,"requests_in_flight":N,"backend_version":"..."}`. If the log file cannot be opened the library exits the process ([issue #13](https://github.com/OAAX-standard/hailo-acceleration/issues/13)): use a writable `log_file`.

## End-to-end example

Goal: the official YOLOX-s model (Apache-2.0) from ONNX to detections on a Hailo-8, with the [YOLOX example](https://github.com/OAAX-standard/examples/tree/main/yolox-example){:target="_blank"} of the examples repository as host. The same host runs every OAAX chip; for Hailo only the model and one flag change.

!!! warning "What was and was not run"
    Steps 1 to 3 and 5 were run on a Hailo-8 (HailoRT 4.20.0, Ubuntu 22.04). **Step 4, the conversion, was run by the example's authors** (about 14 minutes, Dataflow Compiler 3.29.0, HailoRT 4.19.0 wheel), because the Hailo SDK is not redistributable; step 5 used the model they produced.

**Prerequisites**: a Linux x86_64 machine with the Hailo-8 driver and [uv](https://docs.astral.sh/uv/getting-started/installation/); `sudo apt-get install git curl cmake build-essential libgomp1 zip` and [Docker](https://docs.docker.com/engine/install/); and the three SDK files in `hailo-deps/` ([Software](#software)).

1. **Build the host and get the model** (the model's sha256 is checked):

    ```bash
    git clone https://github.com/OAAX-standard/examples.git && cd examples/yolox-example
    cmake -S . -B build && cmake --build build
    bash fetch_model.sh                       # models/yolox_s.onnx
    ```

2. **Get the runtime and the toolchain**, with the variant equal to your HailoRT version:

    ```bash
    uvx oaax-manager fetch runtime hailo --variant hailort4.20.0
    uvx oaax-manager fetch toolchain hailo && docker load -i oaax-hailo-toolchain-2.0.0.tar
    ```

3. **Pack the model** with the options of the example:

    ```bash
    mkdir -p hailo/input hailo/output && cp models/yolox_s.onnx hailo/input/ && cp configs/hailo.json hailo/input/yolox_s.json
    (cd hailo/input && zip yolox_s.zip yolox_s.onnx yolox_s.json)
    ```

4. **Convert** (about 14 minutes; the container runs as root):

    ```bash
    docker run --rm -e PIP_FIND_LINKS=https://storage.googleapis.com/jax-releases/jax_releases.html \
      -v "$PWD/hailo-deps:/app/hailo-deps" -v "$PWD/hailo/input:/app/input:ro" -v "$PWD/hailo/output:/app/output" \
      onnx-to-hailo:latest /app/input/yolox_s.zip /app/output
    sudo chown -R "$(id -u):$(id -g)" hailo/output
    ```

    The last message of `hailo/output/logs.json` must be `Successful Conversion`.

5. **Run the host** with the converted model (`input0` is the renamed input):

    ```bash
    ./build/yolox_host runtime-hailo-2.0.0-linux-x86_64-hailort4.20.0/libRuntimeLibrary.so hailo/output/yolox_s.hailo8.onnx \
      artifacts/dog-bicycle.jpg --input-name input0
    ```

**What you should see**: `Runtime: OAAX Hailo Runtime 2.0.0`, an output tensor of shape `[1, 8400, 85]`, two detections (`bicycle` and `dog`, scores 0.953 and 0.934) and `OK: 10 requests`, then `detections.png`, the picture with the boxes. The unconverted model on the [CPU runtime](CPU.md) gives the same classes and scores, with boxes within about 11 pixels (INT8). As an example, the Hailo-8 took about 12 ms per request and the CPU about 80 ms on the test machine; yours will differ.

## Troubleshooting

| Symptom | Cause | Fix |
| --- | --- | --- |
| `Failed to create ORT session for replica 0`; `hailort.log` says `Driver version (4.20.0) is different from library version (4.19.0)` (`HAILO_INVALID_DRIVER_VERSION(76)`) | Runtime variant differs from the driver | Download the variant equal to the driver version ([how to check](#software)). |
| Same, with `HAILO_DRIVER_FAIL(36)` / `Failed to query driver info` | Same mismatch (seen with a 4.17.0 library on a 4.20.0 driver) | Same fix. |
| Same, `hailortcli scan` lists nothing, no `/dev/hailo*` | Driver not installed or loaded | Install `hailort-pcie-driver`, reload or reboot, check `lsmod \| grep hailo` and the permissions of `/dev/hailo*`. |
| Requests accepted, no output, `NO_OUTPUT_AVAILABLE`; log has `Invalid Feed Input Name:<name>` | Input name differs from the converted model's | Use the name inside the `.hailo8.onnx` (`input0` in the example). |
| `Unexpected input data type` in the log | Tensor type differs from the model's | Send the types the converted model declares. |
| Model loads, but it is slow and the chip is idle | An unconverted ONNX was loaded: it runs on the CPU ([issue #11](https://github.com/OAAX-standard/hailo-acceleration/issues/11)) | Load the `.hailo8.onnx`; it has a `HailoOp` node. |
| `libatomic.so.1` or `libgomp.so.1: cannot open shared object file` | Missing system library | `sudo apt-get install libatomic1 libgomp1`. |
| `MLDataType for: tensor(float) is not currently registered` after switching runtimes | A runtime build older than 2.0.0 | Use the 2.0.0 builds. |
| Conversion fails or exits strangely | See [Outputs, logs and errors](#outputs-logs-and-errors) | Includes the `jaxlib` and missing-`libhailort` cases. |
| Conversion: SDK parse error on a YOLO11 or other new model | Operators the SDK version does not support | Use a supported model or a newer SDK, or cut the graph with `start_node_names` / `end_node_names`. |
