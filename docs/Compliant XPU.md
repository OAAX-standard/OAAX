# Compliant AI Accelerators

These accelerators have an OAAX 2.0 implementation: a runtime library and a conversion toolchain. Open a chip's page for its requirements, downloads, options and a complete example.

| Device class | Hardware | Page | Run end to end by the documentation authors |
| ------------ | -------- | ---- | ------------------------------------------- |
| CPU | x86_64 with AVX2 (Intel, AMD), aarch64 with NEON (Arm) | [CPU](CPU.md) | x86_64 Linux. Not run: aarch64, Windows |
| CPU | Intel CPUs with AVX2 | [Intel](Intel.md) | Yes |
| GPU | Intel GPUs | [Intel](Intel.md) | Integrated GPU |
| GPU | NVIDIA GPUs with CUDA 11, 12 or 13, Jetson, DGX Spark | [NVIDIA](NVIDIA.md) | RTX A4000: ONNX Runtime and TensorRT 10. Not run: Jetson, DGX Spark, Windows, TensorRT 11 |
| NPU | Intel NPU | [Intel](Intel.md) | Not run |
| NPU | Hailo-8 | [Hailo](Hailo.md) | Runtime on Hailo-8 hardware (HailoRT 4.20.0). Conversion needs Hailo's SDK: run by the example's authors, not re-run for the page |
| NPU | DEEPX DX-M1, DX-H1 | [DEEPX](DEEPX.md) | Model conversion. Inference not run (no card) |

A listed chip means an OAAX 2.0 runtime and toolchain are published for it. The last column is what was actually run while writing these pages; each page lists the details.

Fetch any runtime or toolchain with [OAAX Manager](index.md#oaax-manager-get-a-runtime-or-toolchain-in-one-command), then follow the [Hello World example](Getting%20Started.md).

Want your accelerator on this list? See the [Contributing Guide](https://github.com/OAAX-standard/OAAX/blob/main/CONTRIBUTING.md) and the [Specification](Specification.md).
