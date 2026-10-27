> **📦 Implementation details**  
> You can find the implementation of the Hailo runtime & toolchain in the [Github repository](https://github.com/OAAX-standard/hailo-acceleration){:target="_blank"}.

## Overview

OAAX provides a runtime and a conversion toolchain for [Hailo-8](https://hailo.ai/products/ai-accelerators/){:target="_blank"} AI accelerators. The toolchain translates, quantizes and compiles an ONNX model for the chip and writes a Hailo ONNX file (`<model>.hailo8.onnx`); the runtime loads that file and runs inference on the accelerator through ONNX Runtime and HailoRT.

## Requirements

- A Hailo-8 accelerator on Ubuntu (x86_64 or aarch64) with the HailoRT PCIe driver installed. The runtime build must match the HailoRT version you installed (4.17.0 to 4.20.0).
- Docker, to run the conversion toolchain.
- For conversion, the Hailo SDK packages of the same version, which you download from the [Hailo Developer Zone](https://hailo.ai/developer-zone/){:target="_blank"}: `hailo_dataflow_compiler-<version>-py3-none-linux_x86_64.whl`, `hailort-<version>-cp310-cp310-linux_x86_64.whl` and `hailort_<version>_amd64.deb`. They are not part of the toolchain image.

## Usage

### Runtime Library

The runtime library implements the OAAX's runtime interface for initializing, loading models, running inference and cleaning up the runtime.
It is initialized by calling `RuntimeStatus runtime_init(Config config);`, with the following parameters passed as string key/value pairs in `Config`:

- `log_level` (string, default is "1"): The minimal log level: `0` debug, `1` info, `2` warning, `3` error.
- `log_file` (string, default is "runtime.log"): The file to which the runtime logs will be written.
- `n_threads` (string, from "1" to "16", default is "4"): The total number of ONNX Runtime threads, split evenly across all model replicas.
- `n_replicas` (string, default is "1"): The number of ONNX Runtime sessions (worker threads) per model.

Each model has an input queue holding 100 items. When it is full, the oldest item is dropped to make room, so `runtime_enqueue_input` still succeeds: retrieve outputs as fast as you enqueue.

> You can check out the examples repository for more details on how to use the runtime library: [OAAX Examples](https://github.com/OAAX-standard/examples).

### Conversion Toolchain

Pack the model with an `options.json` into a zip archive (calibration images in `calib/*.jpg` are optional; built-in images are used if absent):

```json
{"start_node_names": [], "end_node_names": [], "height": 640, "width": 640, "channels": 3,
  "nchw": false, "means": [0.0, 0.0, 0.0], "stds": [255.0, 255.0, 255.0]}
```

Leave `end_node_names` empty for YOLO-family models: the toolchain finds the cut before the DFL decode head, which the Hailo SDK does not support. Keep `nchw` set to `false`: Hailo expects NHWC calibration data whatever the model's input format.

```bash
docker load -i oaax-hailo-toolchain.tar
docker run --rm -v /path/to/hailo-deps:/app/hailo-deps -v /path/to/input:/app/input:ro -v /path/to/output:/app/output \
  onnx-to-hailo:latest /app/input/model.zip /app/output
```

`/path/to/hailo-deps` is the directory with the three Hailo SDK packages from the requirements. The converted model (`<model>.hailo8.onnx`) and a conversion log (`logs.json`) are written to the output directory.

## Download links and compatibility matrix

Runtime libraries are packaged as `library-<HailoRT version>.tar.gz` and contain `libRuntimeLibrary.so`, the Hailo execution provider for ONNX Runtime and the matching `libhailort`. Sizes are in brackets.

### OAAX 2.0

Conversion toolchain (any platform): [Download](https://nbg1.your-objectstorage.com/oaax/conversion-toolchain/2.0.0/HAILO/oaax-hailo-toolchain.tar) (1.2 GB)

| OS | CPU architecture | HailoRT | Runtime library |
| -- | ---------------- | ------- | --------------- |
| Ubuntu | x86_64 | 4.17.0 | [Download](https://nbg1.your-objectstorage.com/oaax/runtimes/2.0.0/HAILO/x86_64/Ubuntu/library-4.17.0.tar.gz) (9 MB) |
| Ubuntu | x86_64 | 4.18.0 | [Download](https://nbg1.your-objectstorage.com/oaax/runtimes/2.0.0/HAILO/x86_64/Ubuntu/library-4.18.0.tar.gz) (9 MB) |
| Ubuntu | x86_64 | 4.19.0 | [Download](https://nbg1.your-objectstorage.com/oaax/runtimes/2.0.0/HAILO/x86_64/Ubuntu/library-4.19.0.tar.gz) (9 MB) |
| Ubuntu | x86_64 | 4.20.0 | [Download](https://nbg1.your-objectstorage.com/oaax/runtimes/2.0.0/HAILO/x86_64/Ubuntu/library-4.20.0.tar.gz) (9 MB) |
| Ubuntu | aarch64 | 4.17.0 | [Download](https://nbg1.your-objectstorage.com/oaax/runtimes/2.0.0/HAILO/aarch64/Ubuntu/library-4.17.0.tar.gz) (8 MB) |
| Ubuntu | aarch64 | 4.18.0 | [Download](https://nbg1.your-objectstorage.com/oaax/runtimes/2.0.0/HAILO/aarch64/Ubuntu/library-4.18.0.tar.gz) (8 MB) |
| Ubuntu | aarch64 | 4.19.0 | [Download](https://nbg1.your-objectstorage.com/oaax/runtimes/2.0.0/HAILO/aarch64/Ubuntu/library-4.19.0.tar.gz) (9 MB) |
| Ubuntu | aarch64 | 4.20.0 | [Download](https://nbg1.your-objectstorage.com/oaax/runtimes/2.0.0/HAILO/aarch64/Ubuntu/library-4.20.0.tar.gz) (9 MB) |
