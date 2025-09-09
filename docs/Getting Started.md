# Getting Started with OAAX

To deploy an AI model on an OAAX-compliant AI Accelerator, the first step is to ensure that the model is compatible with the AI accelerator. That can be achieved by checking their documentation or attempting to convert the model using their conversion toolchain. If the conversion is successful, the next step is to ensure that the runtime-hosting machine is properly set up with the necessary software and dependencies. Refer to the documentation of the XPU to validate the setup.

That said, to illustrate the OAAX workflow, let's consider the example of running the YOLOv8 model from Ultralytics on Intel CPU, NVIDIA GPU, and DEEPX.

## Requirements

- An x86_64 machine where the model conversion will take place.
- A machine (or machines) with the AI accelerators installed.

## Deployment steps

The following steps outline the process of deploying the YOLOv8 model on Intel CPU, NVIDIA GPU, and DEEPX using OAAX:

1. Export the model from PyTorch to ONNX.
2. Download the conversion toolchain for the target XPUs: Intel, NVIDIA, DEEPX.
3. Convert the model using the conversion toolchain for each XPU.
4. Set up the runtime-hosting machines by installing the necessary OAAX runtime libraries and dependencies.
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

You can download the conversion toolchain for each XPU from their respective repositories or using the links below:

- [Intel toolchain](https://oaax.nbg1.your-objectstorage.com/conversion-toolchain/latest/INTEL/oaax-intel-toolchain.tar)
- [NVIDIA toolchain](https://oaax.nbg1.your-objectstorage.com/conversion-toolchain/latest/NVIDIA/oaax-nvidia-toolchain.tar)
- [DEEPX toolchain](https://oaax.nbg1.your-objectstorage.com/conversion-toolchain/latest/DEEPX/oaax-deepx-toolchain.tar)

To download them on Linux, you can use the following commands:
```bash
wget https://oaax.nbg1.your-objectstorage.com/conversion-toolchain/1.1.1/INTEL/oaax-intel-toolchain.tar
wget https://oaax.nbg1.your-objectstorage.com/conversion-toolchain/1.1.1/NVIDIA/oaax-nvidia-toolchain.tar
wget https://oaax.nbg1.your-objectstorage.com/conversion-toolchain/1.0.0/DEEPX/oaax-deepx-toolchain.tar
```

Now, you can load them into Docker using the following commands:
```bash
docker load -i oaax-intel-toolchain.tar
docker load -i oaax-nvidia-toolchain.tar
docker load -i oaax-deepx-toolchain.tar
```
### Convert the model

Both NVIDIA and Intel toolchains can take in the ONNX model as input directly. However, the DEEPX toolchain requires the input to be an archive (.zip) containing the ONNX model, a JSON configuration file, and calibration images to perform quantization.

> Make sure the ONNX model is an directory named "model" containing the `yolov8n.onnx` file.

You can use the following commands to perform the conversion for each XPU:

#### Intel

```bash
docker run --rm -v ./model:/tmp/run oaax-intel-toolchain:1.1.1 /tmp/run/yolov8n.onnx /tmp/run/output/
```

The output ONNX will be located in the `./model/output/` directory.

#### NVIDIA

```bash
docker run --rm -v ./model:/tmp/run oaax-nvidia-toolchain:1.1.1 /tmp/run/yolov8n.onnx /tmp/run/output/
```

#### DEEPX



### Set up the runtime-hosting machines

Please refer to the documentation of each XPU to validate the setup of the runtime-hosting machines.

### Run the model

Please use the example in the examples repository.