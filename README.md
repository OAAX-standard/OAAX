<p align="center">
<picture>
  <source media="(prefers-color-scheme: dark)" width="60%" srcset="https://raw.githubusercontent.com/OAAX-standard/OAAX/74de2ec57606be7886acaf8dd11cc54a07add5c8/media/logo%20-%20white.png">
  <source media="(prefers-color-scheme: light)" width="60%" srcset="https://raw.githubusercontent.com/OAAX-standard/OAAX/_media/media/logo%20-%20black.png">
  <img alt="Open AI Accelerator eXchange (OAAX)" src="https://raw.githubusercontent.com/OAAX-standard/OAAX/_media/media/logo%20-%20black.png">
</picture>
</p>

[![OpenSSF Best Practices](https://www.bestpractices.dev/projects/10352/badge)](https://www.bestpractices.dev/projects/10352)

# The Open AI Accelerator eXchange (OAAX) Standard

OAAX is an open standard for running (trained) AI models on any AI accelerator (CPU, GPU, NPU, VPU, ... or "XPU")
through one runtime interface. Convert a model once per chip, load that chip's runtime, and the application code stays
the same. OAAX is a Linux Foundation AI & Data project, initiated by [Network Optix](https://www.networkoptix.com/).

This repository holds the **specification**, the **documentation site** ([docs.oaax.org](https://docs.oaax.org)), the
runtime **interface header** and a minimal **illustrative example**. The runtimes and toolchains for each chip live in
the other repositories of the [OAAX-standard organization](https://github.com/OAAX-standard).

## How it works

Deploying a model is a two-stage pipeline:

1. An **OAAX toolchain** (a Docker image) converts an ONNX model into the format the XPU needs: validation, parsing,
   optimization, conversion.
2. An **OAAX runtime** (a shared library) loads the converted model and runs inference on the XPU, through the same C API
   for every chip:

```c
RuntimeStatus runtime_init(Config config);
RuntimeStatus runtime_load_models(int num_models, const ModelConfig *model_configs);
RuntimeStatus runtime_enqueue_input(int model_id, Tensors *input_tensors);
RuntimeStatus runtime_retrieve_output(int *model_id, Tensors **output_tensors, int timeout_ms);
RuntimeStatus runtime_cleanup(void);
```

The full interface is in [include/interface.h](include/interface.h) and is specified in the
[specification](https://docs.oaax.org/Specification/).

## Where to start

| I want to... | Go to |
| --- | --- |
| Run a model on an accelerator | [Getting Started](https://docs.oaax.org/Getting%20Started/), then the accelerator's page on [docs.oaax.org](https://docs.oaax.org) |
| Call a runtime from my own application | The [C example](https://github.com/oaax-standard/examples/tree/main/c-example) in the examples repository |
| Implement OAAX for my chip | The [specification](https://docs.oaax.org/Specification/), the [reference implementation](https://github.com/oaax-standard/reference-implementation) and the [illustrative example](example/README.md) (the three contracts, no real inference) |
| Check which accelerators are supported | [Compliant AI Accelerators](https://docs.oaax.org/Compliant%20XPU/) |

## Repository layout

| Path | Content |
| --- | --- |
| [docs](docs) | Source of the documentation site (MkDocs), including the specification |
| [include/interface.h](include/interface.h) | The OAAX 2.0 runtime interface |
| [example](example) | Illustrative toolchain, runtime and host, built and run by CI |
| [paper](paper) | The position paper, written for the 1.x API (the current API is described in the [specification](https://docs.oaax.org/Specification/)) |

## Contributing

Contributions are welcome: examples, XPU toolchains and runtimes, documentation fixes. See
[CONTRIBUTING.md](CONTRIBUTING.md); every commit needs a DCO sign-off (`git commit -s`). If you are an XPU manufacturer
and want to add your XPU, start from the [reference implementation](https://github.com/oaax-standard/reference-implementation).

OAAX is governed by its steering committee; meeting notes and the roadmap are in the
[steering-committee](https://github.com/OAAX-standard/steering-committee) repository. To join the community, visit
[oaax.org](https://oaax.org).

For the founding motivations, see the [introduction](https://www.networkoptix.com/blog/2024/03/05/introducing-the-open-ai-accelerator-standard).

## License

OAAX is licensed under the [Apache 2.0 License](LICENSE).
