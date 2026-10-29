# Hello World Example

This walkthrough takes a face detection model through the two OAAX stages on a CPU and on an Intel accelerator:

1. **Convert** the ONNX model with the chip's conversion toolchain (a Docker image).
2. **Run** the converted model with the chip's runtime library, through a small C host application.

The same host application runs both, only the runtime library and the model file change. That is the point of OAAX.

For other accelerators, follow the same steps with the toolchain and runtime listed on the accelerator's page ([NVIDIA](NVIDIA.md), [Hailo](Hailo.md), [DEEPX](DEEPX.md), ...), which list the OAAX 2.0 builds and how to pick one.

## Requirements

- A Linux x86_64 machine (Ubuntu 22.04 or higher) with `docker`, `git`, `zip`, `cmake`, a C compiler and libjpeg:
  `sudo apt-get install cmake build-essential libjpeg-dev zip`
- For Intel GPU or NPU inference, the Intel drivers (see the [Intel page](Intel.md#installation)).

## 1. Get the host application and the model

The [examples repository](https://github.com/OAAX-standard/examples) contains a C program that loads any OAAX 2.0 runtime, runs a model on an image and prints the detections, together with a face detection model and an image.

```bash
git clone https://github.com/OAAX-standard/examples.git
cd examples/c-example
mkdir build && (cd build && cmake .. && make)
mkdir -p work/model && cp artifacts/model.onnx work/model/
cd work
```

## 2. CPU

Download the [CPU toolchain and runtime](CPU.md#download-links-and-compatibility-matrix), or fetch them with [`oaax-manager`](#download-and-check-with-the-oaax-tools), then convert the model and run it:

```bash
docker load -i oaax-cpu-toolchain.tar
docker run --rm --user $(id -u):$(id -g) -v ./model:/model oaax-cpu-toolchain:2.0.0 /model/model.onnx /model/cpu-output

mkdir cpu-runtime && tar xzf library.tar.gz -C cpu-runtime    # the CPU runtime archive
../build/c_example cpu-runtime/libRuntimeLibrary.so model/cpu-output/model-simplified.onnx ../artifacts/image.jpg
```

## 3. Intel

Download the [Intel toolchain and runtime](Intel.md#download-links-and-compatibility-matrix). The Intel toolchain takes a `.zip` bundle; the `config.json` keeps the model's FP32 inputs (by default the toolchain converts the model to FP16 and the runtime then expects FP16 input tensors):

```bash
docker load -i oaax-intel-toolchain.tar
mkdir -p intel-input intel-output
cp model/model.onnx intel-input/
echo '{ "optimization": { "fp16_compression": false } }' > intel-input/config.json
(cd intel-input && zip model.zip model.onnx config.json)
docker run --rm --user $(id -u):$(id -g) -v ./intel-input:/input -v ./intel-output:/output oaax-intel-toolchain:latest /input/model.zip /output

mkdir intel-runtime && tar xzf library.tar.gz -C intel-runtime    # the Intel runtime archive
../build/c_example intel-runtime/libRuntimeLibrary.so intel-output/model.zip ../artifacts/image.jpg
```

(Extract the CPU and Intel runtime archives separately: both are named `library.tar.gz`.)

## What you should see

```
Runtime: OAAX CPU Runtime 2.0.0
...
  face 1.000 at (0.142, 0.421) - (0.253, 0.633)
OK: 10/10 requests answered in 7.0 ms (0.7 ms per request)
```

The Intel run prints `Runtime: OAAX Intel 2.0.0` instead. The program exits with a non-zero code if a request fails, and the runtime writes `runtime.log` in the current directory.

## Download and check with the OAAX tools

Two small command-line tools, run with [uv](https://docs.astral.sh/uv/) (no install needed):

- **[oaax-manager](https://pypi.org/project/oaax-manager/)** ([GitHub](https://github.com/OAAX-standard/oaax-manager)) downloads a runtime or toolchain for your machine and verifies its checksum:

  ```bash
  uvx oaax-manager list                    # what is available (* = matches this machine)
  uvx oaax-manager fetch runtime cpu       # writes runtime-cpu-2.0.0-linux-x86_64/libRuntimeLibrary.so
  uvx oaax-manager fetch toolchain cpu     # saves oaax-cpu-toolchain-2.0.0.tar and prints the `docker load` command
  ```

  In [step 2](#2-cpu), use these two paths in place of `oaax-cpu-toolchain.tar` and `cpu-runtime/libRuntimeLibrary.so`.

- **[oaax-conformance](https://pypi.org/project/oaax-conformance/)** ([GitHub](https://github.com/OAAX-standard/conformance)) checks a runtime or toolchain against the specification:

  ```bash
  uvx --python 3.12 --from 'oaax-conformance[e2e]' oaax-conformance runtime ./libRuntimeLibrary.so --model @tiny-conv --out report
  uvx --python 3.12 --from 'oaax-conformance[e2e]' oaax-conformance toolchain oaax-cpu-toolchain:2.0.0 --out report
  ```

  Open `report/report.html` for the details. Exit code 0 means every required check passed. The reference CPU runtime and toolchain have [known findings](https://github.com/OAAX-standard/conformance#known-findings-on-the-reference-implementation), so exit code 1 is expected for them.

## Next steps

- Read how the host talks to a runtime: the [c-example README](https://github.com/OAAX-standard/examples/tree/main/c-example) and the [runtime specification](Specification.md#oaax-runtime-specification).
- Use your own model: export it to ONNX (for example `yolo export model=yolov8n.pt format=onnx` with [Ultralytics](https://github.com/ultralytics/ultralytics)), convert it with the toolchain, and adapt the pre- and post-processing in the host application.
- Add your own accelerator: see the [specification](Specification.md) and the [reference implementation](https://github.com/OAAX-standard/reference-implementation).
