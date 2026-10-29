> **📦 Implementation details**  
> The CPU runtime & toolchain are implemented in the [Github repository](https://github.com/OAAX-standard/reference-implementation){:target="_blank"}. Latest release: [v2.0.0](https://github.com/OAAX-standard/reference-implementation/releases/tag/v2.0.0){:target="_blank"} (checksums are in its release notes).

## At a glance

| | |
| --- | --- |
| **Device class** | CPU |
| **Hardware** | Linux x86_64 (AVX2) and aarch64, Windows 10/11 x86_64 |
| **Engine** | [ONNX Runtime](https://onnxruntime.ai){:target="_blank"} on the CPU (`runtime_get_info()` reports version 1.17.3) |
| **Toolchain input / output** | an `.onnx` file / `<name>-simplified.onnx` + `logs.json` |
| **Runtime library** | `libRuntimeLibrary.so` (Linux), `RuntimeLibrary.dll` (Windows), inside `library.tar.gz` |
| **Toolchain image** | `oaax-cpu-toolchain:2.0.0` (Docker, x86_64, 835 MB `.tar`) |
| **OAAX version / license** | 2.0.0 / Apache-2.0 |
| **Start here** | `uvx oaax-manager fetch runtime cpu && uvx oaax-manager fetch toolchain cpu`, then the [end-to-end example](#end-to-end-example) |

## Overview

The CPU implementation is the baseline OAAX runtime and toolchain. It runs on any supported CPU, needs no driver, and is the quickest way to try OAAX or to start your own accelerator implementation.

- **Toolchain:** loads the ONNX model and simplifies it with [ONNX Simplifier](https://github.com/daquexian/onnx-simplifier){:target="_blank"} (checks the graph, folds constants, removes unused nodes; the result is checked against the original on 3 random inputs). It does **not** quantize, change the precision, rename or reshape the inputs and outputs, or compile for a device. If simplification fails, the original model is kept and the failure is logged.
- **Runtime:** loads the `.onnx` file (or the model bytes from memory) into ONNX Runtime and runs it asynchronously on worker threads.
- **Tensors:** the runtime passes your input tensors to ONNX Runtime as they are, using the tensor names declared in the ONNX graph. Data types (float, float16, int8, uint8, ..., float8), batch sizes and dynamic dimensions are whatever the model and ONNX Runtime accept; the runtime does not check them before inference (see [bad input](#runtime-library)).

## Requirements

### Hardware

| Needed to run | Needed to convert |
| --- | --- |
| x86_64 CPU with AVX2 (the library is built with `-march=haswell`), or an aarch64 CPU. Memory: each model is loaded once per worker replica (see `num_replicas`). | A x86_64 machine that runs Docker. No GPU, no accelerator. |

### Software

| Needed to run | Needed to convert |
| --- | --- |
| **Linux:** glibc 2.29 or higher (Ubuntu 20.04+, Debian 11+), `libstdc++6`, and on x86_64 `libgomp1` (`sudo apt-get install libgomp1`). The CI tests the runtime on Ubuntu 22.04. | [Docker](https://docs.docker.com/engine/install/){:target="_blank"} on x86_64, and about 1.7 GB of disk (835 MB `.tar` + 809 MB image). |
| **Windows:** 10 or 11, x86_64. The Visual C++ runtime DLLs are shipped in the archive. | The image bundles Python 3.11, ONNX 1.21.0, ONNX Simplifier 0.4.36 and ONNX Runtime 1.21.1. |

!!! note "Also needed for the end-to-end example"
    [`uv`](https://docs.astral.sh/uv/getting-started/installation/){:target="_blank"} runs `oaax-manager`: `curl -LsSf https://astral.sh/uv/install.sh | sh`, then `export PATH="$HOME/.local/bin:$PATH"`. To build the host on a minimal image: `sudo apt-get install -y git curl cmake build-essential libgomp1 ca-certificates`.

## Get the runtime and toolchain

There is a single build per platform (no variants), so no variant needs to be picked.

=== "oaax-manager"

    ```bash
    uvx oaax-manager list                    # all builds; * marks this machine's runtime
    uvx oaax-manager fetch runtime cpu       # writes runtime-cpu-2.0.0-linux-x86_64/libRuntimeLibrary.so
    uvx oaax-manager fetch toolchain cpu     # saves oaax-cpu-toolchain-2.0.0.tar
    docker load -i oaax-cpu-toolchain-2.0.0.tar
    ```

    Use `--os linux|windows` and `--arch x86_64|aarch64` to fetch the runtime of another platform. SHA-256 sums are verified for you.

=== "Manual"

    ```bash
    curl -LO https://nbg1.your-objectstorage.com/oaax/runtimes/2.0.0/CPU/x86_64/Ubuntu/library.tar.gz
    echo "f3b71a075b0db7bbc164ca8a3029e719e71a3bcf896ba29232e150ee7a9bc920  library.tar.gz" | sha256sum -c
    mkdir cpu-runtime && tar xzf library.tar.gz -C cpu-runtime      # -> cpu-runtime/libRuntimeLibrary.so
    ```

    The other checksums are in the [release notes](https://github.com/OAAX-standard/reference-implementation/releases/tag/v2.0.0){:target="_blank"}.

### Download links and compatibility matrix

| OAAX version | OS name | OS version      | CPU architecture | Runtime library | Conversion toolchain |
| ------------ | ------- | --------------- | ---------------- | --------------- | -------------------- |
| 2.0.0        | Ubuntu  | 20.04 or higher | x86_64           | [Download](https://nbg1.your-objectstorage.com/oaax/runtimes/2.0.0/CPU/x86_64/Ubuntu/library.tar.gz) | [Download](https://nbg1.your-objectstorage.com/oaax/conversion-toolchain/2.0.0/CPU/oaax-cpu-toolchain.tar) |
| 2.0.0        | Ubuntu  | 20.04 or higher | aarch64          | [Download](https://nbg1.your-objectstorage.com/oaax/runtimes/2.0.0/CPU/aarch64/Ubuntu/library.tar.gz) | ⬆️ |
| 2.0.0        | Windows | 10, 11          | x86_64           | [Download](https://nbg1.your-objectstorage.com/oaax/runtimes/2.0.0/CPU/x86_64/Windows/library.tar.gz) | ⬆️ |

## Conversion toolchain

The image takes **one `.onnx` file** (not a `.zip` bundle) and an output directory. It has no other option, no `config.json` and no environment variable.

```bash
docker run --rm --user "$(id -u):$(id -g)" -v "$PWD/model:/model" oaax-cpu-toolchain:2.0.0 /model/model.onnx /model/output
```

| Argument | Meaning |
| --- | --- |
| `<onnx-path>` | Path **inside the container** of a file ending in `.onnx`. |
| `<output-directory>` | Path inside the container; created if missing. |
| `--user "$(id -u):$(id -g)"` | The image runs as user 1000 (`appuser`); without this flag it can only write to directories that user owns. |
| `-v "$PWD/model:/model"` | Mounts the host directory holding the model. It must be writable (not `:ro`). |

!!! warning "The input directory is written to"
    The toolchain first saves `<model-name>-simplified.onnx` next to the input file, then copies it to the output directory. Expect one extra file in the directory you mounted.

**Output** in `<output-directory>`: `<model-name>-simplified.onnx` (the model to give to the runtime) and `logs.json`, an array of `{"Message": ..., "Data": {...}}` entries:

??? note "`logs.json` entries and fields"
    | Message | `Data` fields |
    | --- | --- |
    | `Simplifying ONNX model` | `ONNX Path`, `MD5`, `Size (bytes)` |
    | `Simplified ONNX model successfully` | `Nodes before`, `Nodes after`, `Duration (s)` |
    | `Failed to simplify ONNX model` (instead of the previous one) | `Error`, `Fallback` (the original file is used) |
    | `Successful Conversion` (always last) | `Output Directory`, `Output file name`, `MIME type` (`application/x-onnx; device=cpu`), `Output file MD5`, `Output size (bytes)`, `Logs file name` |

!!! note
    `Successful Conversion` is also written when simplification failed and the original model was kept: look for `Failed to simplify ONNX model` in `logs.json`. If the toolchain stops with an error, no `logs.json` is written.

| Error (exit code) | Meaning |
| --- | --- |
| `Usage: ... <onnx-path> <output-directory>` (1) | Not exactly two arguments. |
| `error: File not found: <path>` (2) | The path does not exist **inside the container**: check the `-v` mount. |
| `error: --onnx-path must point to a .onnx file` (2) | Wrong extension, for example a `.zip`. |
| `PermissionError: [Errno 13] Permission denied` / `Read-only file system` (1) | Missing `--user $(id -u):$(id -g)`, or a read-only mount. |
| `DecodeError: ... Wire format was corrupt` (1) | The file is not a valid ONNX model. |

## Runtime library

Extract `libRuntimeLibrary.so` (or `RuntimeLibrary.dll`) and load it from the host application: see the [specification](Specification.md#oaax-runtime-specification) and the [C example](https://github.com/OAAX-standard/examples/tree/main/c-example){:target="_blank"}. Config keys and values are strings; unknown keys (including `num_threads`, which 2.0.0 no longer reads) are ignored.

### `runtime_init` keys

| Key | Values | Default | Meaning |
| --- | --- | --- | --- |
| `log_level` | `0` trace, `1` debug, `2` info, `3` warning, `4` error, `5` critical, `6` off | `2` | Minimal level, for the console and the file. Anything else falls back to `2`. |
| `log_file` | path | `runtime.log` | Log file, relative to the working directory. |
| `perf_mode` | `eco`, `power` | `eco` | CPU budget: `eco` uses 40% and `power` 90% of the logical cores (at least 1). |
| `num_intra_threads` | integer >= 0 | `0` (4) | ONNX Runtime threads per inference. `0` means 4. |
| `num_replicas` | integer >= 0 | `0` (derived) | Parallel copies of each model, i.e. inferences in flight. `0` means `max(1, budget / intra threads)`. |

The thread budget per model is `max(1, floor(cores * 0.4 or 0.9) / number of models)`. On a 20-thread machine, `eco` gives a budget of 8, so 2 replicas x 4 threads; `power` gives 18, so 4 replicas x 4 threads. Each replica holds its own copy of the model, so memory grows with `num_replicas`. Higher throughput needs more replicas; lower latency per request needs more intra threads.

`ModelConfig.config` is **not read**: there are no per-model keys. Use `file_path`, or `model_data` + `model_size` to load from memory (if `model_size` is 0, `file_path` is used).

### Behavior

| Topic | Behavior |
| --- | --- |
| Logging | Written to stdout and to `log_file` (rotates at 5 MB, keeps 3 files). Every request failure is logged at `error` level. |
| Models | Load all models in **one** `runtime_load_models` call; a model's id is its index. A failed load leaves nothing loaded and you may call it again. |
| Concurrency | `runtime_enqueue_input` and `runtime_retrieve_output` can be called from different threads. All models share one output queue; `model_id` tells which model answered. |
| Ordering | With more than one replica, outputs can arrive in any order: match them to requests with `Tensors.id`. |
| Limits | Each model queues about 1,000 pending requests (measured: the first failure came at about 1,150 with 1 replica); then `runtime_enqueue_input` returns `ERROR` (1) with `input queue full`. `runtime_retrieve_output(timeout_ms)`: negative waits forever, `0` does not wait. |
| Diagnostics | `runtime_get_info()` returns `{"loaded_models":N,"requests_in_flight":N,"backend_version":"..."}` (queued requests, not those running). |
| Cleanup | Always call `runtime_cleanup()` before exiting or unloading: the process aborts at exit otherwise ([known issue #45](https://github.com/OAAX-standard/reference-implementation/issues/45){:target="_blank"}). |

### Status codes and bad input

??? note "Status codes by call"
    | Call | Status (code) and cause |
    | --- | --- |
    | `runtime_init` | `ALREADY_INITIALIZED` (3) when called twice; `INVALID_ARGUMENT` (5) for a bad `perf_mode`, or a non-integer or negative `num_intra_threads` / `num_replicas`; `ERROR` (1) if the log file cannot be opened. |
    | `runtime_load_models` | `NOT_INITIALIZED` (2); `ALREADY_INITIALIZED` (3) on a second call; `INVALID_ARGUMENT` (5) for `n <= 0`; `ERROR` (1) for every model problem (missing file, invalid ONNX, no path given): read `runtime_get_error()`. |
    | `runtime_enqueue_input` | `NOT_INITIALIZED` (2), `MODEL_NOT_LOADED` (4), `INVALID_MODEL_ID` (18), `INVALID_ARGUMENT` (5) for a null pointer, `ERROR` (1) when the model's queue is full. |
    | `runtime_retrieve_output` | `NO_OUTPUT_AVAILABLE` (12) when nothing arrived before the timeout. |

!!! warning "Wrong tensors are not rejected"
    `runtime_enqueue_input` returns success for a tensor with the wrong name, data type or shape. The inference then fails inside ONNX Runtime, the error is written to the log (for example `Unexpected input data type. Actual: (tensor(float16)), expected: (tensor(float))`), and **no output is ever produced** for that request ([known issue #46](https://github.com/OAAX-standard/reference-implementation/issues/46){:target="_blank"}). A host that only sees `NO_OUTPUT_AVAILABLE` should check the log.

## End-to-end example

Runs the official YOLOX-S detector (36 MB, 80 COCO classes, no NMS inside) from `.onnx` to detections with the generic [YOLOX example](https://github.com/OAAX-standard/examples/tree/main/yolox-example){:target="_blank"} host, which is the same for every chip. Needs Ubuntu 22.04+ on x86_64, Docker and the tools above. It downloads about 880 MB (835 MB toolchain, 36 MB model, 8.5 MB runtime) and uses 809 MB more for the Docker image. Run in a clean Ubuntu 22.04 container, the whole sequence took 35 s on a fast connection.

1. Build the host and fetch the model (checked against a pinned sha256):

    ```bash
    git clone https://github.com/OAAX-standard/examples.git
    cd examples/yolox-example
    cmake -S . -B build && cmake --build build
    bash fetch_model.sh                          # writes models/yolox_s.onnx
    ```

2. Download the runtime and the toolchain, and load the toolchain image:

    ```bash
    uvx oaax-manager fetch runtime cpu
    uvx oaax-manager fetch toolchain cpu
    docker load -i oaax-cpu-toolchain-2.0.0.tar      # prints: Loaded image: oaax-cpu-toolchain:2.0.0
    ```

3. Convert the model (2 s of simplification):

    ```bash
    mkdir -p model && cp models/yolox_s.onnx model/
    docker run --rm --user "$(id -u):$(id -g)" -v "$PWD/model:/model" oaax-cpu-toolchain:2.0.0 /model/yolox_s.onnx /model/output
    ls model/output      # logs.json  yolox_s-simplified.onnx
    ```

4. Run inference:

    ```bash
    ./build/yolox_host runtime-cpu-2.0.0-linux-x86_64/libRuntimeLibrary.so model/output/yolox_s-simplified.onnx artifacts/dog-bicycle.jpg
    ```

Expected output (timings depend on your CPU; here a 20-thread machine in use):

```
Runtime: OAAX CPU Runtime 2.0.0
Runtime info: {"loaded_models":1,"requests_in_flight":0,"backend_version":"1.17.3"}
Input: 'images' float32 NCHW, 1280x854 image letterboxed to 640
Output tensor 'output': type 1, shape [1, 8400, 85]
Detections (score >= 0.30, NMS IoU 0.45): 2
  bicycle        0.953  ( 616,  210) - (1228,  806)
  dog            0.934  ( 216,  244) - ( 521,  587)
Wrote detections.png
OK: 10 requests, first 93.9 ms (warm-up), then 104.5 ms on average (min 89.0 ms)
```

`detections.png` is the picture shown on the [Hello World](Getting%20Started.md) page. Simplification only removed 4 of the 279 nodes (the CPU runtime also runs the unconverted model). Pass options of the table above to the host with `--config KEY=VALUE`, for example `--config perf_mode=power`. The Windows and aarch64 runtimes were not run for this page (not verified on hardware).

## Troubleshooting

| Symptom | Cause and fix |
| --- | --- |
| `Permission denied: '/model/...'` when converting | Run the container as yourself: `--user $(id -u):$(id -g)`, and do not mount the model directory read-only. |
| `File not found: /model/...` | The path is read inside the container; it must start with the mount target (`/model`). |
| `must point to a .onnx file` | The CPU toolchain takes the `.onnx` itself, not a `.zip`. |
| `DecodeError`, or an empty output file after converting an empty `.onnx` | The input is not a valid ONNX model. Re-export it. An empty file is "converted" to an empty file, and the runtime then fails to load it (`ModelProto does not have a graph`). |
| `runtime_init` returns 5 | See `runtime_get_error()`: `perf_mode must be "eco" or "power"`, or `num_intra_threads and num_replicas must be ...`. |
| `runtime_init` returns 1 with `Logger initialization failed` | The folder of `log_file` does not exist or is not writable. |
| `runtime_load_models` returns 1 | Read `runtime_get_error()`: `File doesn't exist`, `ModelProto does not have a graph`, or `file_path and model_data are both null`. |
| Requests succeed but no output arrives | Check the log for `Inference error for request N`: a wrong tensor name (`Invalid input name`), data type (`Unexpected input data type`, for example float16 sent to a float32 model) or shape (`Got invalid dimensions`). |
| `libgomp.so.1: cannot open shared object file` or `GLIBC_2.29 not found` | Install `libgomp1`; the OS is older than Ubuntu 20.04 / Debian 11. (From the library's dependencies; not reproduced.) |
| `Illegal instruction` on x86_64 | The CPU has no AVX2. Not verified: no such CPU was available. |
| The host aborts when it exits | `runtime_cleanup()` was not called ([#45](https://github.com/OAAX-standard/reference-implementation/issues/45){:target="_blank"}). |
| Changing `num_threads` has no effect | 2.0.0 reads `perf_mode`, `num_intra_threads` and `num_replicas` instead. |
