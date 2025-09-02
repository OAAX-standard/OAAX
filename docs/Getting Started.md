# Getting Started with OAAX

To deploy an AI model on an OAAX-compliant AI Accelerator, the first step is to ensure that the model is compatible with the AI accelerator. That can be achieved by checking their documentation or attempting to convert the model using their conversion toolchain. If the conversion is successful, the next step is to ensure that the runtime-host machine is properly set up with the necessary software and dependencies. Refer to the documentation of the XPU to validate the setup.

To illustrate the OAAX workflow, let's consider an example of running the nano YOLOv8 model from Ultralytics on Intel CPU, NVIDIA GPU, and DEEPX.

## Requirements

- An x86_64 machine where the model conversion will take place.
- A machine (or machines) with the AI accelerators installed.

## Yolov8n deployment steps

1. Export the model from PyTorch to ONNX.
2. Download the conversion toolchain for the target XPUs: Intel, NVIDIA, DEEPX.
3. Convert the model using the conversion toolchain for each XPU.
4. Set up the runtime-host machines by installing the necessary OAAX runtime libraries and dependencies.
5. Run the model on each XPU using the appropriate OAAX runtime.


### Exporting the model to ONNX
- Optionally, create a separate Python virtual environment for this process.
```bash
python3 -m venv .venv
source .venv/bin/activate
```
- Install the [Ultralytics](https://github.com/ultralytics/ultralytics) using:
```bash
pip install ultralytics
```
- Install the ONNX package
```bash
pip install onnx==1.15.0
```
- Export the model to ONNX format using the following command:
```bash
yolo export model=yolov8n.pt format=onnx
```

This process will generate a file named `yolov8n.onnx` that we'll later use as a source model for the conversion toolchains.

### Download the conversion toolchain

Please refer to the documentation of each XPU to download their conversion toolchain.

### Convert the model

Please refer to the documentation of each XPU for instructions on how to convert the ONNX model to their specific format.

### Set up the runtime-host machines

Please refer to the documentation of each XPU to validate the setup of the runtime-host machines.

### Run the model

Please use the example in the examples repository.