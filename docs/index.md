# Welcome to OAAX
![OAAX](./media/logo%20-%20black%20text.png#only-light){ width="100%" }
![OAAX](./media/logo%20-%20white%20text.png#only-dark){ width="100%" }

**OAAX (Open AI Accelerator eXchange)** is an open standard that lets one ONNX model run on many AI accelerators through one conversion step and one runtime API. Export your model from any framework to ONNX, convert it with the accelerator's toolchain, and load it from your application through the same C interface on every chip. No framework or vendor specific integration.

OAAX is an open-source, community-driven project of the LF AI & Data Foundation.

## Pick your accelerator

Every page covers the same things: requirements, where to get the runtime and the toolchain, every option, and a complete example you can copy.

<div class="grid cards" markdown>

-   :material-cpu-64-bit: **CPU**

    ---

    x86_64 and aarch64, Linux and Windows. The reference implementation: nothing to install but Docker.

    [:octicons-arrow-right-24: CPU](CPU.md)

-   :material-expansion-card: **NVIDIA** (GPU)

    ---

    CUDA GPUs, Jetson and DGX Spark. ONNX Runtime or native TensorRT backend.

    [:octicons-arrow-right-24: NVIDIA](NVIDIA.md)

-   :material-chip: **Intel** (CPU, GPU, NPU)

    ---

    OpenVINO based: Intel CPUs, GPUs and the NPU.

    [:octicons-arrow-right-24: Intel](Intel.md)

-   :material-brain: **Hailo** (NPU)

    ---

    Hailo-8, one runtime build per HailoRT version.

    [:octicons-arrow-right-24: Hailo](Hailo.md)

-   :material-brain: **DEEPX** (NPU)

    ---

    DX-M1 and DX-H1 on Ubuntu, x86_64 and aarch64.

    [:octicons-arrow-right-24: DEEPX](DEEPX.md)

-   :material-rocket-launch: **Run your first model**

    ---

    Convert a face detection model and run it on a CPU or an Intel accelerator in about ten minutes.

    [:octicons-arrow-right-24: Hello World](Getting%20Started.md)

</div>

See the [full list of compliant hardware](Compliant%20XPU.md).

## OAAX Manager: get a runtime or toolchain in one command

**What it is for.** Every accelerator ships a *runtime library* and a *conversion toolchain*, in several builds (CPU architecture, CUDA or HailoRT version, Ubuntu version). [`oaax-manager`](https://github.com/OAAX-standard/oaax-manager) finds the right one for your machine, downloads it, checks its checksum and unpacks it, so you never hunt for download links.

**How to use it.** It needs [uv](https://docs.astral.sh/uv/getting-started/installation/) and nothing else; run it without installing anything:

```bash
uvx oaax-manager list                       # everything available; * marks what fits this machine
uvx oaax-manager fetch runtime cpu          # downloads and unpacks the runtime for this machine
uvx oaax-manager fetch toolchain cpu        # saves the toolchain image and prints the `docker load` command
```

Replace `cpu` with `intel`, `nvidia-ort`, `nvidia-trt`, `hailo` or `deepx`. When a chip has several builds, it never guesses: it lists them and you choose with `--variant`.

| Command or option | What it does |
| ----------------- | ------------ |
| `list [--kind runtime\|toolchain] [--vendor V]` | Show the available runtimes and toolchains |
| `fetch runtime <vendor>` | Download, verify and extract the runtime for this OS and CPU |
| `fetch toolchain <vendor>` | Download and verify the toolchain Docker image (`.tar`) |
| `--variant V` | Pick a build when several exist (for example `cuda12`, `hailort4.20.0`) |
| `--os`, `--arch` | Fetch a runtime for another platform (for example `--arch aarch64`) |
| `--artifact-version X.Y.Z` | Pin an OAAX version (default: the newest) |
| `-o DIR` | Where to write |

!!! success "Safe by default"
    Every download is checked against a pinned SHA-256, interrupted downloads resume, archives are extracted all-or-nothing with strict path checks, and an existing folder is never overwritten.

Install it permanently with `uv tool install oaax-manager`, or use the single file from the [releases page](https://github.com/OAAX-standard/oaax-manager/releases) with any Python 3.9+. Details: [PyPI](https://pypi.org/project/oaax-manager/), [GitHub](https://github.com/OAAX-standard/oaax-manager).

## How OAAX works

1. **Convert** the ONNX model with the accelerator's toolchain. The result is an OAAX bundle for that chip.
2. **Load** the accelerator's runtime library in your application, and call `runtime_init(config)`.
3. **Load the model** with `runtime_load_models(num_models, model_configs)`. A model is referred to by its index in `model_configs` (its `model_id`).
4. **Exchange data asynchronously.**
    1. Enqueue inputs with `runtime_enqueue_input(model_id, input_tensors)`, setting a unique `id` on each `Tensors`.
    2. Retrieve outputs with `runtime_retrieve_output(&model_id, &output_tensors, timeout_ms)`. The output carries the `id` of the input it answers.
5. **Clean up** with `runtime_cleanup()`.

The application code is the same for every chip: only the runtime library and the model file change. See the [specification](./Specification.md#oaax-runtime-specification) and the [interface header](https://github.com/OAAX-standard/OAAX/blob/main/include/interface.h) for the full API.

??? info "Terminology"
    - **OAAX**: Open AI Accelerator eXchange, a standard (specification, recipe) for deploying AI models across different hardware accelerators.
    - **XPU**, **AI accelerator** or **AI hardware**: Any processing unit that can execute AI models, such as CPUs, GPUs, NPUs or specialized AI accelerators.
    - **Compile** or **Convert**: Turning an ONNX model into a format optimized and supported for a specific XPU.
    - **Runtime**: The shared library (`.so` or `.dll`) that implements the OAAX runtime interface to interact with an XPU.
    - **Conversion toolchain**: The software that compiles ONNX models.
    - **Input/Output tensors**: Data structures (`Tensors`) that hold the input and output data for the model.
    - **Host** or **runtime host**: The software that uses a runtime to offload model computation to the AI hardware.
    - **OAAX-compliant XPU**: Any AI accelerator with an implementation of the OAAX standard, both the conversion toolchain and the runtime library.

## Learn more

- [Hello World example](./Getting%20Started.md): the shortest path from an ONNX model to detections.
- [OAAX Specification](./Specification.md): the runtime and toolchain contract.
- [Compliant AI accelerators](./Compliant%20XPU.md): which hardware is supported.
- [FAQ](./FAQ.md): common questions.
- [Contributing Guide](https://github.com/OAAX-standard/OAAX/blob/main/CONTRIBUTING.md): add an implementation for your chip, improve an existing one, write an example or propose a change to the standard.
