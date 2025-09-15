# Hello World Example

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

You can download the conversion toolchain for each XPU from their respective documentation page or using the links below:

- [Intel toolchain](https://oaax.nbg1.your-objectstorage.com/conversion-toolchain/latest/INTEL/oaax-intel-toolchain.tar)
- [NVIDIA toolchain](https://oaax.nbg1.your-objectstorage.com/conversion-toolchain/latest/NVIDIA/oaax-nvidia-toolchain.tar)
- [DEEPX toolchain](https://oaax.nbg1.your-objectstorage.com/conversion-toolchain/latest/DEEPX/oaax-deepx-toolchain.tar)

To download them on Linux, you can use the following commands:
```bash
wget https://oaax.nbg1.your-objectstorage.com/conversion-toolchain/latest/INTEL/oaax-intel-toolchain.tar
wget https://oaax.nbg1.your-objectstorage.com/conversion-toolchain/latest/NVIDIA/oaax-nvidia-toolchain.tar
wget https://oaax.nbg1.your-objectstorage.com/conversion-toolchain/latest/DEEPX/oaax-deepx-toolchain.tar
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

The conversion can be initiated using the following command:
```bash
docker run --rm -v ./model:/tmp/run oaax-intel-toolchain:1.1.1 /tmp/run/yolov8n.onnx /tmp/run/intel-output/
```

The output ONNX will be located in the `./model/intel-output/yolov8n-simplified.onnx` directory.

#### NVIDIA

The conversion can be initiated using the following command:
```bash
docker run --rm -v ./model:/tmp/run oaax-nvidia-toolchain:1.1.1 /tmp/run/yolov8n.onnx /tmp/run/nvidia-output/
```
The output ONNX will be located in the `./model/output/yolov8n-simplified.onnx` directory.

#### DEEPX

To prepare the zip archive, we'll need to use the following JSON configuration file. Save it as `deepx-config.json` in the `./model` directory.
<details> 
<summary>deepx-config.json</summary>
```json
{
  "inputs": {
    "images": [
      1,
      3,
      640,
      640
    ]
  },
  "calibration_num": 100,
  "calibration_method": "ema",
  "train_batchsize": 32,
  "num_samples": 1024,
  "default_loader": {
    "dataset_path": "/app/dataset",
    "file_extensions": [
      "jpeg",
      "jpg",
      "png",
      "JPEG"
    ],
    "preprocessings": [
      {
        "resize": {
          "mode": "pad",
          "size": 640,
          "pad_location": "edge",
          "pad_value": [
            114,
            114,
            114
          ]
        }
      },
      {
        "div": {
          "x": 255
        }
      },
      {
        "convertColor": {
          "form": "BGR2RGB"
        }
      },
      {
        "transpose": {
          "axis": [
            2,
            0,
            1
          ]
        }
      },
      {
        "expandDim": {
          "axis": 0
        }
      }
    ]
  }
}
```
</details>

In addition, you'll need to prepare a dataset of images for calibration. Save the images in a directory named `dataset` inside the `./model` directory.

Now, you can create the zip archive using the following command:
```bash
cd model
zip -r deepx-yolov8n.zip yolov8n.onnx deepx-config.json dataset
```

The next step is to download the DX COM SDK from DEEPX developer portal, since it's not included by default in the conversion toolchain. You can download it from [here](https://developer.deepx.ai/).
Once it is downloaded and unpacked next to the `./model` directory, you can run the conversion using the following command:
```bash
docker run --rm -v ./model:/app/artifacts -v ./dx_com:/app/dx_com oaax-deepx-toolchain:latest /app/artifacts/deepx-yolov8n.zip /app/artifacts/deepx-output/
```

The compiled model will be located at `./model/deepx-output/yolov8n.dxnn`.

#### Summary
At this point, you should have the following files inside the `./model` directory:

- `yolov8n.onnx`: The original ONNX model exported from Ultralytics framework.
- `intel-output/yolov8n-simplified.onnx`: The optimized ONNX model for Intel CPUs.
- `nvidia-output/yolov8n-simplified.onnx`: The optimized ONNX model for NVIDIA GPUs.
- `deepx-output/yolov8n.dxnn`: The compiled model for DEEPX AI accelerator.

### Set up the runtime-hosting machines

This step involves installing the necessary driver and SDK libraries for each AI accelerator on the respective runtime-hosting machines. Please refer to the official documentation of each XPU for detailed instructions on how to set up the environment.

> Note: Ensure you have hardware that is supported by the respective XPU, and that you install the supported versions of the drivers and SDKs.

### Run the model

Finally, the last step is to run the model on each XPU using the appropriate runtime. Please consult each XPU's page for the appropriate runtime library to use based on your hardware and software environment:

- [Intel runtime](./Intel.md#download-links-and-compatibility-matrix){:target="_blank"}
- [NVIDIA runtime](./NVIDIA.md#download-links-and-compatibility-matrix){:target="_blank"}
- [DEEPX runtime](./DEEPX.md#download-links-and-compatibility-matrix){:target="_blank"}

After downloading the appropriate runtime, you can use the [YOLOv8 inference example]([../examples/yolov8-inference/README.md](https://github.com/OAAX-standard/examples/tree/main/yolov8-inference)){:target="_blank"} to run the model on each XPU. Make sure to update the paths in the `build-and-run.sh` script accordingly.

On a successful run, you should see an output similar to the following if you're using the NVIDIA runtime:
```
[2025-09-15 20:20:01.932] [OAAX] [info] Initializing OAAX inference engine with the following parameters:
[2025-09-15 20:20:01.932] [OAAX] [info] Library Path: ./artifacts/libRuntimeLibrary.so
[2025-09-15 20:20:01.932] [OAAX] [info] Model Path: ./model/nvidia-output/yolov8n-simplified.onnx
[2025-09-15 20:20:01.932] [OAAX] [info] Input Path: ../artifacts/image.jpg
[2025-09-15 20:20:01.932] [OAAX] [info] Configuration Path: ../model-configs/nvidia-model.json
[2025-09-15 20:20:01.932] [OAAX] [info] Log File: app.log
[2025-09-15 20:20:01.932] [OAAX] [info] Log Level: 2
[2025-09-15 20:20:01.943] [OAAX] [info] Runtime Name: OAAX NVIDIA Runtime
[2025-09-15 20:20:01.943] [OAAX] [info] Runtime Version: 1.1.1
[2025-09-15 20:20:01.945] [OAAX NVIDIA Runtime] [info] Initializing the runtime
[2025-09-15 20:20:02.032] [OAAX] [info] Runtime initialized successfully.
[2025-09-15 20:20:02.032] [OAAX NVIDIA Runtime] [info] Inference thread started
[2025-09-15 20:20:02.032] [OAAX NVIDIA Runtime] [info] Runtime arguments:
[2025-09-15 20:20:02.032] [OAAX NVIDIA Runtime] [info]   log_level: 2
[2025-09-15 20:20:02.032] [OAAX NVIDIA Runtime] [info]   log_file: runtime.log
[2025-09-15 20:20:02.032] [OAAX NVIDIA Runtime] [info]   num_threads: 4
[2025-09-15 20:20:02.032] [OAAX NVIDIA Runtime] [info] Loading model from: ./model/nvidia-output/yolov8n-simplified.onnx
[2025-09-15 20:20:02.083] [OAAX] [info] Model loaded successfully: ./model/nvidia-output/yolov8n-simplified.onnx
[2025-09-15 20:20:02.083] [OAAX] [info] Configuration: {
    "model": {
        "input_dtype": "float32",
        "input_height": 640,
        "input_name": "images",
        "input_width": 640,
        "mean": [
            0,
            0,
            0
        ],
        "nchw": 1,
        "std": [
            255,
            255,
            255
        ]
    }
}
[2025-09-15 20:20:02.083] [OAAX] [info] Preprocessing image: ../artifacts/image.jpg
[2025-09-15 20:20:02.087] [OAAX] [info] Mean: 0, 0, 0
[2025-09-15 20:20:02.087] [OAAX] [info] Stddev: 255, 255, 255
[2025-09-15 20:20:02.090] [OAAX] [info] Image min value: 0, max value: 1
[2025-09-15 20:20:02.090] [OAAX] [info] Creating tensors for input image: images
Number of tensors: 1
Tensor id=0:
  Name: 'images'
  Data type: 1
  Rank: 4
  Shape: 1 3 640 640 
  Data Pointer: 0x7f2b5574e010

[2025-09-15 20:20:02.092] [OAAX] [info] Starting input sending and output receiving threads...
[2025-09-15 20:20:02.092] [OAAX] [info] Sending input tensors to the runtime...
[2025-09-15 20:20:02.092] [OAAX] [info] Waiting for threads to finish...
[2025-09-15 20:20:02.093] [OAAX] [info] Sent input tensors: 1
[2025-09-15 20:20:02.095] [OAAX] [info] Sent input tensors: 2
[2025-09-15 20:20:02.096] [OAAX] [info] Sent input tensors: 3
[2025-09-15 20:20:02.098] [OAAX] [info] Sent input tensors: 4
[2025-09-15 20:20:02.100] [OAAX] [info] Sent input tensors: 5
[2025-09-15 20:20:02.101] [OAAX] [info] Sent input tensors: 6
[2025-09-15 20:20:02.103] [OAAX] [info] Sent input tensors: 7
[2025-09-15 20:20:02.105] [OAAX] [info] Sent input tensors: 8
[2025-09-15 20:20:02.107] [OAAX] [info] Sent input tensors: 9
[2025-09-15 20:20:02.108] [OAAX] [info] Sent input tensors: 10
[2025-09-15 20:20:02.108] [OAAX] [info] All input tensors sent successfully.
[2025-09-15 20:20:03.093] [OAAX] [info] Output tensors received: 1
[2025-09-15 20:20:03.093] [OAAX] [info] Output tensors received: 2
[2025-09-15 20:20:03.093] [OAAX] [info] Output tensors received: 3
[2025-09-15 20:20:03.093] [OAAX] [info] Output tensors received: 4
[2025-09-15 20:20:03.093] [OAAX] [info] Output tensors received: 5
[2025-09-15 20:20:03.093] [OAAX] [info] Output tensors received: 6
[2025-09-15 20:20:03.093] [OAAX] [info] Output tensors received: 7
[2025-09-15 20:20:03.093] [OAAX] [info] Output tensors received: 8
[2025-09-15 20:20:03.093] [OAAX] [info] Output tensors received: 9
[2025-09-15 20:20:03.093] [OAAX] [info] Terminating OAAX inference engine.
[2025-09-15 20:20:03.093] [OAAX] [info] Output tensors received: 10
[2025-09-15 20:20:03.093] [OAAX] [info] Output tensors received successfully.
[2025-09-15 20:20:03.093] [OAAX] [info] Threads finished successfully.
[2025-09-15 20:20:03.093] [OAAX NVIDIA Runtime] [info] Destroying runtime...
```