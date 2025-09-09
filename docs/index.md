# Welcome to OAAX
<picture>
   <source srcset="./media/logo - white text.png" media="(prefers-color-scheme: dark)" />
   <img src="./media/logo - black text.png" width="100%" alt="ONNX - OAAX" />
</picture>

OAAX serves as a bridge between popular AI frameworks and diverse hardware accelerators. Models developed in frameworks such as TensorFlow, PyTorch, Hugging Face, and others are first exported to the ONNX format, a widely adopted standard for interoperability. OAAX then connects ONNX models to a variety of hardware backends—including CPUs, Intel, NVIDIA, DEEPX, EdgeCortix, Hailo, and more—enabling seamless deployment across heterogeneous compute platforms without requiring framework- or vendor-specific integration.

This is achieved by providing a unified conversion and runtime interfaces, enabling developers to convert ONNX models into hardware-specific formats and run them seamlessly across different platforms using a standardized API.  
In practice, OAAX enables models from frameworks like TensorFlow, PyTorch, Hugging Face, and others—exported via ONNX—to run seamlessly across diverse hardware accelerators such as CPUs, Intel, NVIDIA, DEEPX, EdgeCortix, and Hailo.

## Terminology

Before delving into the OAAX standard, it's important to understand some key terms that are frequently used:

- **OAAX**: Open AI Accelerator eXchange, a standard (specification, recipe) for deploying AI models across different hardware accelerators.
- **XPU**, **AI accelerator** or **AI hardware**: Any processing unit that can execute AI models, such as GPUs, TPUs, or specialized AI accelerators.
- **Compile** or **Convert**: The process of converting an ONNX model into a format optimized for a specific XPU.
- **Runtime**: The library that provides that implements the OAAX runtime interface to interact with the XPU.
- **Conversion Toolchain**: The software that compiles ONNX models.
- **Input/Output Tensors**: Data structures that hold the input and output data for the model.
- **Host** or **Runtime host**: The software that interacts with runtime to offload model computation to the AI hardware.
- **OAAX-compliant XPU**: Refers to any AI accelerator that has an implementation of the OAAX standard, including both the conversion toolchain and the runtime library.

## Usage workflow

To run an AI model on an OAAX-compliant XPU, a typical workflow looks like this:

1. Convert the ONNX model into an XPU-specific OAAX bundle using the provided toolchain.
2. In the host application, load the OAAX runtime library (`libRuntimeLibrary.so`) appropriate to the XPU.
3. Initialize the runtime by calling `runtime_initialization()`.
4. Load the model with `runtime_model_loading("./model.oaax")`.
5. Exchange data asynchronously:
   - Send inputs with `send_input(input_tensors)`.
   - Retrieve outputs with `receive_output(output_tensors_holder)`.
6. When finished, clean up resources by calling `runtime_destruction()`.


## Outline

- To get started with OAAX, please check out the [Getting Started Guide](./Getting%20Started.md).
- To learn about the OAAX specification, please check out the [OAAX Specification Document](./Specification.md).
- To check if an AI Accelerator is compliant with OAAX, please refer to this [list](./Compliant%20XPU.md).
- To contribute a new OAAX implementation, improve an existing one, develop an example or propose new change to the standard, please refer to the [Contributing Guide](https://github.com/OAAX-standard/OAAX/blob/main/CONTRIBUTING.md).
- Have more questions? Please check out the [FAQ](./FAQ.md).