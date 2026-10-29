# Compliant AI Accelerators

These accelerators have an OAAX 2.0 implementation: a runtime library and a conversion toolchain. Open a chip's page for its requirements, downloads, options and a complete example.

| Device class | Hardware | Page |
| ------------ | -------- | ---- |
| CPU | x86_64 with AVX2 (Intel, AMD), aarch64 with NEON (Arm) | [CPU](CPU.md) |
| CPU | Intel CPUs with AVX2 | [Intel](Intel.md) |
| GPU | Intel GPUs | [Intel](Intel.md) |
| GPU | NVIDIA GPUs with CUDA 11, 12 or 13, Jetson, DGX Spark | [NVIDIA](NVIDIA.md) |
| NPU | Intel NPU | [Intel](Intel.md) |
| NPU | Hailo-8, Hailo-8L | [Hailo](Hailo.md) |
| NPU | DEEPX DX-M1, DX-H1 | [DEEPX](DEEPX.md) |

Fetch any runtime or toolchain with [OAAX Manager](index.md#oaax-manager-get-a-runtime-or-toolchain-in-one-command), then follow the [Hello World example](Getting%20Started.md).

Want your accelerator on this list? See the [Contributing Guide](https://github.com/OAAX-standard/OAAX/blob/main/CONTRIBUTING.md) and the [Specification](Specification.md).
