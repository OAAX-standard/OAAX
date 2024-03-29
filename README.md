> The OAXS description is currently work in progress. Please join the conversation and help democratize edge AI at [https://oaxs.org](https://oaxs.org).


# The Open AI Accelerator Standard (OAXS)

This repository (and other linked repositories under the OAX-standard organization) contain documentation and reference implementations, as well as contributed implementations, of the Open AI Accelerators Standard (OAXS).

## Overview

* [About OAXS](#About-OAXS)
	* [The aims of OAXS](#The-aims-of-OAXS)
	* [The scope of OAXS](#The-scope-of-OAXS)
	* [High level implementation](#High-level-implementation)
* [Using OAXS](#Using-OAXS)
	* [Getting started from c](#Getting-started-from-c)
	* [Getting started from python](#Getting-started-from-python)
* [OAXS developement](#OAXS-developement)
	* [Contributing to OAXS](#Contributing-to-OAXS)
	* [The OAXS steering group](#The-OAXS-steering-group)
* [OAXS documentation](#OAXS-documentation)
	* [Toolchain documentation](#Toolchain-documentation)
	* [Runtime documentation](#Runtime-documentation)
* [OAXS contributed impelementations](#OAXS-contributed-impelementations)
	
# About OAXS

OAXS intends to provide a simple to use, easy to expand, *standardized* method of adopting specific (edge) AI accelerators (we will call them "XPUs") into edge AI applications. 

The OAXS standard is designed to make it easy to take a *trained* AI model and execute it (i.e., generate inferences) on novel (edge) AI hardware. For AI solution developers the OAXS standard should make it easy to reap the benefits of the new chipsets that are becoming available without having to worry about the target hardware when setting up and testing their initial AI pipeline. For those designing and bringing to the market new chipsets (NPUs, FPUs, GPUs, etc.) OAXS is aimed to lower the barriers of usage of their products and it provides a unified way in which -- if adhered to -- any developer can easily access the advantages or the novel hardware. 

For our initial introduction to OAXS and its founding motivations, please see the [founding white-paper](https://www.networkoptix.com/blog/2024/03/05/introducing-the-open-ai-accelerator-standard).


## The aims of OAXS

For **AI solution developers**, OAXS aims to...

* ...Prodivde a unified way of *converting* a trained AI/Ml model specified in a standardized generic format (we use [ONNX](https://onnx.ai) as our standard way of expressing a model) to a specific format that run on the supported target hardware.
* ...Provide a unified way of *executing* the specific model on the target hardware. I.e., OAXS provides a standardized ABI/API to pass input data to an AI/ML model, run inference, and retreive the model out.

Jointly, the above should allow AI solutions developers to easily move between different hardware targets and reap the fruits of new developments in accelerator design that benefit their use case.

For **manufacturers of new (edge) AI hardware**, OAXS aims to ...

* ...Provide a unified way of allowing "access" to their hardware. By adhering to OAXS there is a single point of entry for developers looking to embrace new hardware.
* ...Reduce the need for ones own software ecosystem. By adhering to OAXS highger level software tools and ecosystems can easily incorporate new hardware without the need for the manufacturer of the accelerator to provide all the parts of a full AI/ML pipeline: accelerator manufacturers can simply focus on their hardware and the core software components needed to run an AI model on their hardware.

Note that OAXS is designed for maximum flexbility for the AI hardware manufacturer to adopt the trained AI model to their preferred format and to -- if desirable -- keep toolchains or other core bits of software that are deemed business valuable proprietary.


## The scope of OAXS

Currently, OAXS focusses on privding a unified method of *accelerating* (i.e., (partly) moving to a NPU, GPU, FPU, XPU, ..) *AI/ML models*. Models in our context models are simply (mathematical) functions that are specified using [ONNX](https://onnx.ai). Note that at is core, ONNX is simply a standardized format used to express which computations (operations) need to be carried out in which order (the graph). We assume that AI/ML models have on or multiple (`n`) typed tensors as input, and  one or multiple (`m`) typed tensors as output. 

OAXS does **not** facilitate using the accelerator for application/vertical specific tasks such as decoding of a video stream or FFT of a vibration sensor; such operations might be performmed well by a specific accelerator / XPU, but they do not fall into to initial OAXS specification. Also, OAXS does **not** concern model training: we assume the trained model is available.

The diagram below provides a high level abstraction of a common edge AI pipeline in which sensor data is pre-processed, fed to an AI or ML model for inference, its results are post-processed, and that final output is visualized. Within OAXS we assume the pipeline runs on a generic CPU (multiple architectures are supported), whereas the AI model (or inidivual operations within the AI model, depending on the accelerator manufactureres choice) are *accelerated* by the XPU using the OAXS standard.

```mermaid
flowchart LR;
    A[Sensor];
    B[Pre-processing];
    C[AI/ML-model];
    style C fill:#f9f,stroke:#333,stroke-width:4px
    D[Post-processing];
    E[Visulazation];
    A-->B
    B-->C;
    C-->D;
    D-->E;
```

## High level implementation

At a very high level, the OAXS specification is surprisingly simple. As a starting point, we assume a (trained) AI model `M` is available in ONNX format. Hence, there is access to a *generic, deployment independent, description of the function that the model implements*. From this starting point we postulate that only the following two steps are neccesary to accelerate an AI/ML model (or parts thereoff) within the larger pipeline:

1. *Conversion* of the model's generic ONNX model specification to a format that can run on the accelerator. Note that the result might itself be in ONNX formate (for example when operations are accelerated one-by-one by the runtime, see below), or the result might be a totally proprietary binary. The core insight is that there is a "conversion" step neccesary from generic model desciption to a model description that is specific and can be (party) accelerated using the XPU of interest. For the conversion step the OAXS standard merely provides a unified way of going from ONNX to the target's specification (whatever this might be) including standardized error handling and formatting. We call the conversion process the **OAXS toolchain.**
2. "Running" the specific model specification, i.e., generating inference. Once a specific model specification is available--after conversion--we simply need a unified way to, from a process running on the CPU, pass data to the model, execute its computation, and return the results. We call this the **OAXS runtime.** Note that the OAXS runtime itself runs on the host CPU and is responsible for all interactions with the XPU.

The above are effectively two seperate "processes" (although, for nuances, see the [OAXS documnentation]() below). The first of the two can be executed anywhere: the conversion of a generic AI model to the target specific model can be done in the cloud, on your local machine, or using some hosted conversion service. The second process runs on the edge device that runs the pipeline (see diagram above).

Given the availability of a standard -- OAXS -- for sharing both steps with the larger developer community, it is easy for developers to adopt a new XPUs into their AI pipeline: a developer simply...

1. ...runs the conversion process to obtain a specific model implementation, and 
2. ..."swaps" the runtime of the AI model for the appropriate XPU version.

Please note that for the toolchains we use pre-defined [docker containers](https://www.docker.com). Usage of Docker allows maximum flexibility for those contributing toolchains and by simply specifying the high level interactions with the container and resulting data we provide a unified method of model conversion while abstracting away any implementation details. For the runtimes we allow any format, as long as there is a unified runtime [ABI]() available such that the runtime can be integrated in `c++` code (we use shared libs as our preferred implementation; see below). 


# Using OAXS

Here we provide a few simple and (relatviely) self contained examples of how to use the OAXS standard to switch from a CPU only implementation of a (simplified) AI pipeline to one in which the model is accelerated using a [Hailo]() Accelerator (using OAXS to move over to this XPU). For the sake of example, we use a simple pre-trained [YoloV5 vision model in ONNX format]().

Note that in all cases we simply present the core functional lines of code for the application (i.e., we present pseudo-code); full running examples can be found in our [examples repository]().

## Getting started from `c++`
> The full example can be found [here]().

Using the ONNX runtime, in `c++`, we can execute the YoloV5 model as follows:

``` c

try {
   outputTensor = session.Run(Ort::RunOptions{ nullptr }, input_node_names->data(), inputTensor.data(), inputTensor.size(), output_node_names.data(), 1);
 }
 catch (Ort::Exception oe) {
   std::cout << "ONNX exception caught: " << oe.what() << ". Code: " << oe.GetOrtErrorCode() << ".\n";
   return result;
 }


```

The above can be converted for use with the Hailo accelerator using our 2 step process:

### 1. Model conversion (CMD)
First, we simply run the supplied docker container to convert our model:

```
> ls

# Step 1, CMD toolchain to  convert file
> docker run hailo-conv-toolchain-container "file.onnx"

> ls
hailo-file.oaxs
conversion-log.json
error-log.json
message.json
```


The conversion process results in a new file, `hailo-file.oaxs` and a meta data triplet (conversion-log, error-log, message.json). You can find details regarding the content and structure of each of these objects in the [OAXS conversion toolchain documentation]() below.


### 2. Updated pipeline
Next, we update the pipeline to "swap" the ONNX runtime for the Hailo runtime:


``` c

input_data =   ....

OAXSRuntime("hailo")
...

```

This completes the inclusion of the Hailo accelerator, using OAXS, into your AI pipeline.


## Getting started from `python`
> The full example can be found [here](). For python we make use of the `OAXSpy` package.

Using the ONNX Runtime, in `python`, we can execute the YoloV5 model as follows:

```
# load the model
onnx = read.modelFile(file.onnx)

# use ONNX runtime
input.tensor = [[12,10],[56,09]]

ONNXRuntime.load(onnx)
inference = ONNXRuntime.run(input.tesor)
print(inference)
```

Using OAXS to include the Hailo XPU into this pipeline again is a two step process:

### 1. Model conversion (CMD)
First, we simply run the supplied docker container to convert our model (this step is the exact same as in the `c++` example above):

```
> ls

# Step 1, CMD toolchain to  convert file
> docker run hailo-conv-toolchain-container "file.onnx"

> ls
hailo-file.oaxs
conversion-log.json
error-log.json
message.json
```


The conversion process results in a new file, `hailo-file.oaxs` and a meta data triplet (conversion-log, error-log, message.json). You can find details regarding the content and structure of each of these objects in the [OAXS conversion toolchain documentation]() below.

### 2. Updated pipeline
Next, we update the pipeline to "swap" the ONNX runtime for the Hailo runtime:

```
# Step 2, Use new runtime (python-like speudo code)

# Data same as previous
input.tensor = [[12,10],[56,09]]

# use Hailo runtime
OAXSRuntime.load(Hailo) # Python helper function to load OAXS runtimes
OAXSRuntime.loadModel('hailo-file.oaxs')

inference = OAXSRuntime(input.tesor)
print(inference)

```

This completes the inclusion of the Hailo accelerator, using OAXS, into your AI pipeline.


# OAXS development

In this section of the OAXS documentation we describe the ways in which you can contribute to growing OAXS or be involved in maturing the standard.

## Contributing to OAXS

The first way to contribute is by creating examples, implementations, or XPU specific OAXS toolchains or runtimes.

> If you are an XPU manufacturer and are looking to add your XPU to OAXS, please see the sections [Creating an OAXS toolchain]() and [Creating an OAXS runtime]() below.

There are many places to get started:

* If you are building example pipelines that utilize OAXS that you would like to share with the larger community, please start with the [Using OAXS]() documentation.
* If you are looking whether a specific XPU supports OAXS, please see our [OAXS contributed impelementations](). 
* Ir you are contributing a novel XPU, please see [Creating an OAXS toolchain]() and [Creating an OAXS runtime]() below, and the templates in the linked repositories. 
* If you are looking to validate and test your implementation, please see our [OAXS testing]() documentation. This section also includes a description of how to formally [add your XPU to the OAXS ecosystem]().


## The OAXS steering group

OAXS was started by [Network Optix]() as an attempt to solve hardware / XPU incompatability which was slowing market adoption of novel hardware breakthroughs. Thus, from its inception, OAXS has not been tied to individual hardware manufacturers: it is a user driven initiative trying to make adoption of *any* new accelerator designs into actual meaningful business applications as easy as possible.

The following people are core to the development of OAXS:

* ....

## Active users
Currently OAXS is used by the following companies and developer to create their edge AI/ML pipelines:

* ... 

# OAXS documentation

We split our documetnation into four parts (which are partly devided over the linked repositories):

1. Using the OAXS standard in your (edge) AI projects.
2. Creating an OAXS toolchain.
3. Creating an OAXS runtime.
4. OAXS testing and contributing.

## Using OAXS

You find docuemtnation and usage examples [here]().

## Toolchain documentation

You will find documentation, reference implementations, and exmaples for creating toolchains [here]().

## Runtime documentation

You will find documentation, reference implementations, and exmaples for creating runtimes [here]().

## OAXS testing

You find test to evaluate your OAXS implementation before submission [here]().

### Contributing your OAXS implementation

As an accelerator manufacturere, once you have implemented your OAXS toolchain and OAXS runtime, and once you have completed the OAXS tests, you can contribute your OAXS impelementation using the following steps:

* ... Create a feature branch of the ..

[ Desribe how to add... ]

After we have tested your contribution and merged into the main branch we will add it to the OAXS contributed implementation below.

# OAXS contributed impelementations

The tabel below lists available OAXS implementations for various XPUs (and other targets).

| **Name** | **Chipset** | **Toolchain** | **ARM32** | **ARM64** | **x86** |
|----------|-------------|---------------|-----------|-----------|---------|
| Template | N/A         | [..]()        | [..]()    | [..]()    | [..]()  |
|          |             |               |           |           |         |
| Hailo    | Hailo-8     | [..]()        | [..]()    | [..]()    | [..]()  |