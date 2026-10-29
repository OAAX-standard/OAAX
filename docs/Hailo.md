> **📦 Implementation details**  
> You can find the implementation of the Hailo runtime & toolchain in the [Github repository](https://github.com/OAAX-standard/hailo-acceleration){:target="_blank"}. This page describes the [latest release (v2.0.0)](https://github.com/OAAX-standard/hailo-acceleration/releases/tag/v2.0.0){:target="_blank"}.

| At a glance | |
| --- | --- |
| **NPU class** | Dedicated AI accelerator on a PCIe / M.2 module |
| **Chips** | Hailo-8 (see the [Hailo-8L note](#overview)) |
| **HailoRT versions** | 4.17.0, 4.18.0, 4.19.0, 4.20.0: one runtime build per version |
| **Engine** | ONNX Runtime with the Hailo execution provider, on top of HailoRT |
| **Model format** | ONNX in, `<model>.hailo8.onnx` out (the compiled model is wrapped in an ONNX file) |
| **Runtime library** | `libRuntimeLibrary.so` (Ubuntu, x86_64 and aarch64) |
| **Toolchain image** | `onnx-to-hailo:latest` (Docker, x86_64 hosts only, 1.2 GB, needs Hailo's SDK files) |

## Overview

- **Toolchain**: unpacks a zip (ONNX model, `options.json`, optional calibration images), then runs the Hailo SDK steps *parse*, *optimize* (quantization, using calibration images) and *compile* for the `hailo8` architecture. It writes `<model>.hailo8.onnx`: an ONNX graph whose `HailoOp` node embeds the compiled model, followed by the parts of the original graph the chip cannot run (for YOLOv8: the box decoding).
- **Runtime**: loads that file with ONNX Runtime; the Hailo execution provider runs the `HailoOp` node on the chip through HailoRT, and everything else on the host CPU. Inputs and outputs keep the types and layout of the converted model's graph (float32 NCHW image in, float32 out for YOLOv8n).
- **One runtime build per HailoRT version**: each archive bundles its own `libhailort`, which must match the HailoRT driver installed on the host ([how to pick](#software)).
- **Model constraints**: the Hailo SDK decides which operators convert; the repository does not list them. It documents YOLOv8n and YOLOv8s as converting, and YOLO11n and YOLO26n as failing (attention layers, with SDK 3.25.0). Models need one image input with a fixed size, given in `options.json`.
- **Not done**: no Hailo-8L target, no model script or optimization level, no batch or device selection, no accuracy check, no pre- or post-processing in the runtime (the host decodes the output). A plain (unconverted) ONNX file also loads, but then runs on the host CPU.

!!! warning "Hailo-8L"
    The toolchain compiles for the `hailo8` architecture only (hard-coded) and the repository documents no Hailo-8L test. Hailo-8 is the verified chip; check a converted model on a Hailo-8L yourself before relying on it.

## Requirements

### Hardware

- A **Hailo-8** module (PCIe or M.2) in an x86_64 or aarch64 host.
- Conversion needs an **x86_64** Linux machine (the toolchain image and Hailo's compiler are x86_64 only). Copy the converted model to an aarch64 device afterwards.

### Software

| Needed to | Requirement |
| --- | --- |
| **Run** | Ubuntu 20.04 or higher (the libraries need glibc 2.29 / libstdc++ from GCC 9; Ubuntu 22.04 was used for this page) |
| | `libatomic1` and `libgomp1` (`sudo apt-get install libatomic1 libgomp1`) |
| | Hailo's PCIe driver (`hailort-pcie-driver`) installed, with `/dev/hailo*` present |
| | A runtime build whose HailoRT version **equals the installed driver version** (below) |
| **Convert** | Docker, and internet access (the container installs the SDK's Python dependencies at every start) |
| | From Hailo's [Developer Zone](https://hailo.ai/developer-zone/){:target="_blank"} (login required, not redistributable): `hailo_dataflow_compiler-3.<minor>.0-py3-none-linux_x86_64.whl`, `hailort-4.<minor>.0-cp310-cp310-linux_x86_64.whl` and `hailort_4.<minor>.0_amd64.deb`, **one version of each** in one folder |

The image itself holds Ubuntu 22.04 and Python 3.10 (hence the `cp310` wheel). The repository documents conversions only with SDK 3.25.0 and HailoRT 4.20.0.

**Find the variant to download**: it is the version of the installed driver.

```bash
hailortcli --version                 # HailoRT-CLI version 4.20.0
dpkg -l | grep -i hailo              # hailort and hailort-pcie-driver, same version
modinfo hailo_pci | grep ^version    # driver version
hailortcli scan                      # lists the devices
```

## Get the runtime and toolchain

[oaax-manager](https://pypi.org/project/oaax-manager/) (use 0.1.1 or newer, which fixed the Hailo checksums; check with `uvx oaax-manager --version`) downloads and verifies both:

```bash
uvx oaax-manager list                                              # Hailo rows: hailort4.17.0 ... hailort4.20.0
uvx oaax-manager fetch runtime hailo --variant hailort4.20.0       # -> runtime-hailo-2.0.0-linux-x86_64-hailort4.20.0/
uvx oaax-manager fetch runtime hailo --variant hailort4.20.0 --arch aarch64   # for an aarch64 device
uvx oaax-manager fetch toolchain hailo                             # -> oaax-hailo-toolchain-2.0.0.tar
```

The runtime folder holds `libRuntimeLibrary.so`, `libonnxruntime_providers_hailo.so`, `libonnxruntime_providers_shared.so`, `libhailort.so.<version>` and `oaax_runtime.h`. **Keep them together**: the companion libraries are found next to `libRuntimeLibrary.so`.

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

1. Put the three SDK files in `hailo-deps/`, and make a zip with the model and `options.json` (below). The calibration images are optional: without them the toolchain uses the 100 COCO images it ships with.
2. Load the image. Its name and tag come from the tar: **`onnx-to-hailo:latest`** (there is no versioned tag).
3. Run it. **Do not pass `--user`**: the container installs the SDK as root (`dpkg`) and fails otherwise.

```bash
docker load -i oaax-hailo-toolchain-2.0.0.tar
docker run --rm -v "$PWD/hailo-deps":/app/hailo-deps -v "$PWD/input":/app/input:ro -v "$PWD/output":/app/output \
  onnx-to-hailo:latest /app/input/model.zip /app/output
sudo chown -R "$(id -u):$(id -g)" output     # the container writes as root
```

The entry point takes exactly two arguments: the zip path and the output folder (both inside the container). Every start re-installs the SDK and takes a while; the conversion itself takes minutes.

### Zip contents

| File | Rule |
| --- | --- |
| `*.onnx` | Exactly one, at the top level. Its name decides the output name. |
| `*.json` | Exactly one, at the top level: the options below. |
| `*.jpg`, `*.jpeg` | Optional calibration images, anywhere in the zip (for example `calib/`). Resized (stretched) to `width` x `height`. |

### `options.json`

| Key | Type | Default | Meaning |
| --- | --- | --- | --- |
| `height`, `width` | int | required | Model input size; calibration images are resized to it. |
| `channels` | int | required | `1` = grayscale calibration images, anything else = RGB. |
| `nchw` | bool | required | Layout of the calibration array. Keep `false`: Hailo expects NHWC data whatever the model's layout. |
| `means`, `stds` | list of numbers | required | Calibration pixels become `(pixel - mean) / std` (pixels are 0 to 255). **Feed the same normalization to the runtime.** One value or one per channel. |
| `start_node_names` | list of strings | `[]` | Where the SDK starts parsing; empty = the model's inputs. |
| `end_node_names` | list of strings | `[]` | Where the SDK stops parsing; the nodes after are kept as plain ONNX on the host. Empty = auto-detect the YOLO head (the `Conv` nodes feeding the DFL decode, which the SDK cannot run); other models: empty = whole graph. |

Other keys are ignored. Example for YOLOv8 at 640 x 640 (values scaled to 0 to 1):

```json
{"start_node_names": [], "end_node_names": [], "height": 640, "width": 640, "channels": 3,
 "nchw": false, "means": [0.0, 0.0, 0.0], "stds": [255.0, 255.0, 255.0]}
```

### Outputs, `logs.json`, errors

The output folder gets `<model>.hailo8.onnx`, `logs.json` and a `tmp/` folder (the unzipped input, safe to delete). `logs.json` is a list of `{"Message": ..., "Data": {...}}`:

| Message | `Data` |
| --- | --- |
| `Converting ONNX to Hailo-ONNX` | `Input Path` |
| `Starting conversion` | `Target Hardware` (`hailo8`) |
| `Auto-detected end nodes` | `nodes`: the cut found for YOLO models |
| `Successful Conversion` (last) | `MIME type` (`application/x-onnx; device=hailo-8`), `Output Path`; stages `Translation`, `Optimizing Model`, `Compiling Model` are `Done` |
| `Conversion Failed` (last) | `Human Error` (always the same generic text), `Technical Error` (the real reason), `Logs path` |

!!! warning "The exit code is 0 even when the conversion fails"
    Check that the last message is `Successful Conversion` or that `<model>.hailo8.onnx` exists. Only a failure before the conversion (see below) gives a non-zero exit code.

| Message | Meaning and fix |
| --- | --- |
| `dpkg: error: cannot access archive '../hailo-deps/hailort_4.*.0_amd64.deb'` (exit 2) | `hailo-deps` not mounted or the file is missing or misnamed. |
| `dpkg: error: requested operation requires superuser privilege` (exit 2) | You passed `--user`. Remove it. |
| `Usage: /app/scripts/convert.sh <onnx-path> <output-directory>` (exit 1) | Wrong number of arguments. |
| Technical Error `No module named 'hailo_sdk_client'` | The SDK wheels were not installed (check the container's log above it). |
| Technical Error `File is not a zip file`, `No ONNX file found in the zip`, `More than one ONNX file found in the zip` (same for JSON) | Fix the zip. |
| Technical Error `'means'` (or another key) | A required `options.json` key is missing. |
| Technical Error from the SDK parser (it may recommend other end nodes) | An operator is not supported. Use the recommended `end_node_names` / `start_node_names`, or another export of the model. |

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

The threads only run the ONNX nodes that stay on the CPU (for example the decode nodes of a converted YOLOv8); the chip works inside HailoRT. There is no device-selection key. HailoRT writes its own `hailort.log` in the working directory.

### Models and inputs

- `ModelConfig`: give `file_path` (`model_data` + `model_size` loads from memory). `ModelConfig.config` is unused. At most **8 models**; a model's id is its index in the array. Models load once per `runtime_init`.
- **Inputs are bound by tensor name**, in any order. The name must be the one in the *converted* model: the Hailo SDK renames the input (for example to `input0`), so read it from the `.hailo8.onnx` file, not from your original model.
- Each model has an input queue and the runtime one output queue, 100 items each. When full, the **oldest** item is dropped and `runtime_enqueue_input` still succeeds: keep retrieving. With several replicas, outputs can come back in any order: match them by `Tensors.id`.
- `runtime_enqueue_input` and `runtime_retrieve_output` can be called from different threads.
- To switch models, call `runtime_cleanup` and `runtime_init` again. The library exports only the OAAX API (checked in CI), so a host can unload it and load another copy.

### Status codes and errors

| Call | Status | When |
| --- | --- | --- |
| `runtime_init` | `ALREADY_INITIALIZED` (3) | Called twice without `runtime_cleanup`. |
| `runtime_load_models` | `NOT_INITIALIZED` (2), `ALREADY_INITIALIZED` (3), `INVALID_ARGUMENT` (5) | No init; models already loaded; fewer than 1 or more than 8 models. |
| | `ERROR` (1) | Any model failure (missing file, unsupported model, chip unreachable, version mismatch). `runtime_get_error()` says `Failed to create ORT session for replica N`; **the reason is in the log** (`ORT error: ...`). |
| `runtime_enqueue_input` | `NOT_INITIALIZED` (2), `MODEL_NOT_LOADED` (4), `INVALID_MODEL_ID` (18), `INVALID_ARGUMENT` (5) | Wrong call order, bad id, `NULL` tensors. |
| `runtime_retrieve_output` | `NO_OUTPUT_AVAILABLE` (12) | Nothing within the timeout (`0` = do not wait, negative = wait forever). |

!!! warning "A bad input is not reported to the caller"
    Wrong tensor name, type or shape is **accepted** by `runtime_enqueue_input`. The request fails inside the worker, is logged (`Inference run failed` and the ONNX Runtime reason) and no output ever arrives: the host just sees `NO_OUTPUT_AVAILABLE`. Read the log.

`runtime_get_info()` returns `{"loaded_models":N,"requests_in_flight":N,"backend_version":"..."}`. If the log file cannot be opened, the library exits the process: use a writable `log_file`.

## End-to-end example

Goal: YOLOv8n from ONNX to detections on a Hailo-8, with the [OAAX C example](https://github.com/OAAX-standard/examples/tree/main/c-example){:target="_blank"} as host.

!!! note "Why not the example's face model"
    The C example's own `model.onnx` has two inputs and the NMS baked into the graph. The toolchain feeds one image input and only documents YOLO-style models, so it is not a reliable conversion candidate; this page does not claim it converts. YOLOv8n is the documented one, and its output needs a small host-side decode, added by the patch in step 6.

!!! warning "What was and was not run"
    Run by the author of this page: steps 1, 2 and 4 to 7, on a Hailo-8 with HailoRT 4.20.0, **using a YOLOv8n `.hailo8.onnx` converted earlier by the maintainers** (SDK 3.25.0, the same options). **Not run**: step 3, the conversion, because the Hailo SDK is not redistributable.

You need `uv`, `docker`, `git`, `zip`, `cmake`, `build-essential` and `libjpeg-dev`, plus the three SDK files in `hailo-deps/`.

1. **Export YOLOv8n** (Ultralytics models are AGPL-3.0: check the license before redistributing them). `opset=12` is what the maintainers' tests use.

    ```bash
    uvx --python 3.12 --index https://download.pytorch.org/whl/cpu --default-index https://pypi.org/simple \
      --index-strategy unsafe-best-match --with onnx --with onnxslim --with onnxruntime \
      --from ultralytics yolo export model=yolov8n.pt format=onnx imgsz=640 opset=12
    ```

2. **Pack it** with the `options.json` shown [above](#optionsjson):

    ```bash
    mkdir -p input output && cp yolov8n.onnx input/
    echo '{"start_node_names": [], "end_node_names": [], "height": 640, "width": 640, "channels": 3, "nchw": false, "means": [0.0, 0.0, 0.0], "stds": [255.0, 255.0, 255.0]}' > input/options.json
    (cd input && zip model.zip yolov8n.onnx options.json && rm yolov8n.onnx options.json)
    ```

3. **Convert** (not run by the page's author; needs the SDK files). Fetch the toolchain with `uvx oaax-manager fetch toolchain hailo`, then run the `docker load` and `docker run` commands from [Run it](#run-it). Check that `output/yolov8n.hailo8.onnx` exists and that the last message of `output/logs.json` is `Successful Conversion`.

4. **Get the runtime** matching your driver ([how to check](#software)):

    ```bash
    uvx oaax-manager fetch runtime hailo --variant hailort4.20.0
    ```

5. **Build the C example**, pinned to the commit the patch was made for:

    ```bash
    git clone https://github.com/OAAX-standard/examples.git && cd examples && git checkout 66fa557
    ```

6. **Adapt it to YOLOv8** (one input named `input0`, 640 x 640, values 0 to 1, and a decode of the `[1, 84, 8400]` output: 4 box values and 80 class scores per anchor, no NMS). Apply the patch and build:

    ??? note "Patch for `c-example/src/main.c`"
        ```bash
        git apply <<'EOF'
        --- a/c-example/src/main.c
        +++ b/c-example/src/main.c
        @@ -27,7 +27,6 @@
         #define MODEL_ID 0  // Index of the model in the array given to load_models
        -#define IMAGE_WIDTH 320
        -#define IMAGE_HEIGHT 240
        -#define IMAGE_MEAN 127.0f
        -#define IMAGE_STD 128.0f
        -#define NMS_SENSITIVITY 0.5f
        +#define IMAGE_WIDTH 640
        +#define IMAGE_HEIGHT 640
        +#define IMAGE_MEAN 0.0f
        +#define IMAGE_STD 255.0f
         
        @@ -39,5 +38,5 @@ static double now_ms(void) {
         
        -// Builds the input tensors of request `id`: the image (NCHW float) and the NMS
        -// sensitivity. The tensor names are those declared in the model's ONNX graph.
        -// The image is copied: once enqueued, the runtime owns and frees the tensors.
        +// The input name must be the one in the model's ONNX graph: "input0" after a
        +// Hailo conversion (set INPUT_NAME to override it, e.g. "images" for the
        +// original model).
         static Tensors *build_input_tensors(int id, const float *image) {
        @@ -46,11 +45,8 @@ static Tensors *build_input_tensors(int id, const float *image) {
               3 * IMAGE_HEIGHT * IMAGE_WIDTH * sizeof(float);  // In bytes
        -  const int nms_shape[] = {1};
        -  const float nms_sensitivity = NMS_SENSITIVITY;
        +  const char *name = getenv("INPUT_NAME") ? getenv("INPUT_NAME") : "input0";
         
        -  Tensors *input = alloc_tensors(id, 2);
        +  Tensors *input = alloc_tensors(id, 1);
           if (input == NULL) return NULL;
        -  if (fill_tensor(&input->tensors[0], "image-", DATA_TYPE_FLOAT, 4, image_shape,
        -                  image, image_size) != 0 ||
        -      fill_tensor(&input->tensors[1], "nms_sensitivity-", DATA_TYPE_FLOAT, 1,
        -                  nms_shape, &nms_sensitivity, sizeof(float)) != 0) {
        +  if (fill_tensor(&input->tensors[0], name, DATA_TYPE_FLOAT, 4, image_shape,
        +                  image, image_size) != 0) {
             free_tensors(input);
        @@ -61,26 +57,27 @@ static Tensors *build_input_tensors(int id, const float *image) {
         
        -// Prints the metadata of an output tensor, and its detections if it holds
        -// `bboxes-format:xyxysc` rows (x1, y1, x2, y2, score, class), where the
        -// coordinates are relative to the image size (0 to 1).
        +// YOLOv8 output [1, 84, 8400]: 4 box values + 80 class scores per anchor.
        +// Prints the first anchors scoring >= 0.5 (no NMS, so neighbours repeat).
         static void print_output(const Tensors *output) {
        -  for (int i = 0; i < output->num_tensors; i++) {
        -    const TensorDescriptor *tensor = &output->tensors[i];
        -    printf("Output tensor '%s': type %d, shape [", tensor->name,
        -           (int)tensor->data_type);
        -    for (int d = 0; d < tensor->rank; d++) {
        -      printf(d ? ", %d" : "%d", tensor->shape[d]);
        -    }
        -    printf("]\n");
        -
        -    if (tensor->data_type != DATA_TYPE_FLOAT || tensor->rank != 2 ||
        -        tensor->shape[1] != 6 ||
        -        tensor->data_size < (size_t)tensor->shape[0] * 6 * sizeof(float)) {
        -      continue;
        -    }
        -    const float *rows = (const float *)tensor->data;
        -    for (int r = 0; r < tensor->shape[0]; r++) {
        -      const float *box = rows + r * 6;
        -      printf("  face %.3f at (%.3f, %.3f) - (%.3f, %.3f)\n", box[4], box[0],
        -             box[1], box[2], box[3]);
        +  const TensorDescriptor *t = &output->tensors[0];
        +  printf("Output tensor '%s': type %d, shape [%d, %d, %d]\n", t->name,
        +         (int)t->data_type, t->rank > 0 ? t->shape[0] : 0,
        +         t->rank > 1 ? t->shape[1] : 0, t->rank > 2 ? t->shape[2] : 0);
        +  if (t->data_type != DATA_TYPE_FLOAT || t->rank != 3 || t->shape[1] != 84)
        +    return;
        +  const float *d = (const float *)t->data;
        +  const int n = t->shape[2];
        +  int shown = 0;
        +  for (int a = 0; a < n && shown < 10; a++) {
        +    int best = 0;
        +    float score = 0;
        +    for (int c = 0; c < 80; c++) {
        +      if (d[(4 + c) * n + a] > score) {
        +        score = d[(4 + c) * n + a];
        +        best = c;
        +      }
             }
        +    if (score < 0.5f) continue;
        +    printf("  class %d score %.2f box cx=%.0f cy=%.0f w=%.0f h=%.0f (pixels)\n",
        +           best, score, d[a], d[n + a], d[2 * n + a], d[3 * n + a]);
        +    shown++;
           }
        EOF
        ```

    ```bash
    cd c-example && mkdir build && (cd build && cmake .. && make)
    ```

7. **Run** (the host app, the runtime and the converted model; paths are relative to `examples/c-example`):

    ```bash
    ./build/c_example <runtime folder>/libRuntimeLibrary.so <path to output>/yolov8n.hailo8.onnx artifacts/image.jpg
    ```

**What you should see**: `Runtime: OAAX Hailo Runtime 2.0.0`, the output tensor `post_output0` with shape `[1, 84, 8400]`, up to ten lines `class 0 score ... box ...` (class `0` is `person` in COCO; the sample image shows a group of people, and neighbouring anchors repeat because there is no NMS), and a final `OK: 10/10 requests answered ...`. The program exits with a non-zero code if a request fails, and the runtime writes `runtime.log.0`.

!!! tip "Check the host on the CPU first"
    The C example ships the [CPU runtime](CPU.md). Running the unconverted model with it, `INPUT_NAME=images ./build/c_example artifacts/libRuntimeLibrary_x86_64.so <path>/yolov8n.onnx artifacts/image.jpg`, gives similar detections and rules out host-side mistakes before touching the chip (use `libRuntimeLibrary_aarch64.so` on aarch64).

## Troubleshooting

| Symptom | Cause | Fix |
| --- | --- | --- |
| `Failed to create ORT session for replica 0`, and `hailort.log` says `Driver version (4.20.0) is different from library version (4.19.0)` (`HAILO_INVALID_DRIVER_VERSION(76)`) | Runtime variant newer or older than the driver | Download the variant equal to the driver version ([how to check](#software)). |
| Same error with `HAILO_DRIVER_FAIL(36)` / `Failed to query driver info` | Same family of mismatch (seen with a 4.17.0 library on a 4.20.0 driver) | Same fix. |
| Same error, `hailortcli scan` lists nothing, no `/dev/hailo*` | Driver not installed or not loaded, or the module is not seated | Install Hailo's `hailort-pcie-driver`, reload or reboot, check `lsmod \| grep hailo` and the file permissions of `/dev/hailo*`. |
| Requests accepted, no output, `NO_OUTPUT_AVAILABLE` after the timeout; log has `Invalid Feed Input Name:<name>` | Input tensor name differs from the converted model's input | Use the name inside the `.hailo8.onnx` (often `input0`, not your original name). |
| `Unexpected input data type` in the log | Tensor type differs from the model's (for example a bool mask sent to a float input) | Send the types the converted model declares. |
| Model loads, no error, but it is slow and the chip is idle | An unconverted ONNX was loaded: it runs on the CPU | Load the `.hailo8.onnx` file. It contains a `HailoOp` node (`onnx.load(path)`, then look at `graph.node`). |
| `libatomic.so.1` or `libgomp.so.1: cannot open shared object file` | Missing system library | `sudo apt-get install libatomic1 libgomp1`. |
| `MLDataType for: tensor(float) is not currently registered` after switching runtimes | A runtime build older than 2.0.0 | Use the 2.0.0 builds. |
| Conversion: any message in [Outputs, `logs.json`, errors](#outputs-logsjson-errors) | | See that table. |
| Conversion: SDK parse error on a YOLO11 or other new model | Operators the SDK version does not support (the repository lists YOLO11n and YOLO26n as failing with SDK 3.25.0) | Use a supported model, a newer SDK, or cut the graph with `start_node_names` / `end_node_names`. |
