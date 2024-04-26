> The OAX Standard initiative is currently work in progress. Please join the conversation and help democratize edge AI at [https://oaxs.org](https://oaxs.org).


# The Open AI Accelerator (OAX) Standard 

This repository (and other linked repositories under the OAX-standard organization) contain documentation and reference 
implementations, as well as contributed implementations, of the Open AI Accelerators (OAX) Standard.

	
# About OAXS

The Open AI Accelerator (OAX) Standard intends to provide a simple to use, easy to expand, *standardized* method of 
adopting specific (edge) AI accelerators such as NPUs, GPUs, FPUs or the like 
-- we will call them XPUs" throughout -- into edge AI applications. 

The OAX standard is designed to make it easy to take a *trained* AI model and use it to run inference on novel (edge) AI hardware.
For AI solution developers the OAX standard should make it easy to reap the benefits of the new chipsets that are 
becoming available rapidly without having to worry about the target hardware when setting up and testing their initial AI pipeline.   
For those designing and bringing to the market new XPUs, OAX Standard is aimed to lower the barriers of entry of 
the hardware by providing a unified way in which -- when adhered to -- any developer can easily utilize the advantages of the newly introduced XPU. 

* For our initial introduction to OAX Standard and its founding motivations, please see the [founding white-paper](https://www.networkoptix.com/blog/2024/03/05/introducing-the-open-ai-accelerator-standard).

* For detailed information about the OAX Standard, including a high-level specification, please see the [position paper](Position%20paper/oaxs-spec-white-paper.pdf).

In summary, within the framework of OAX, the deployment process of an AI model is conceptualized as a two-stage pipeline.   
1. The initial stage encompasses a series of steps and procedures necessary for transforming a trained AI model into an optimized format or binary. 
These steps may include, but are not limited to, validation, parsing, optimizations, and conversion. 
2. The latter phase entails the utilization of the produced model file, and executing it on a designated hardware XPU.
This initial stage is performed by a "Conversion Toolchain," while the subsequent stage is conducted by a "XPU Runtime".

# Using OAX

## Illustrative implementation example
We are providing code examples to illustrate the high-level overview of how to implement 
an OAX-conforming AI pipeline: toolchain and runtime for OAX contributors, and application for OAX users.
> The full source code can be found in the [Illustrative example](Illustrative%20example) folder.

## Full CPU-acceleration example
We are providing a full example of how to implement an OAX-conforming AI pipeline for a CPU-accelerated application. 
For the sake of example, we're using [OnnxRuntime](https://github.com/microsoft/onnxruntime) as the runtime 
and [ONNX](https://github.com/onnx/onnx) as the model format.
> The C/C++ example for using that runtime can be found in the [reference implementation](examples) repository. 
> While, The toolchain and runtime artifacts are available in this [examples]() repository.

# OAXS development

In this section of the OAX documentation, we describe the ways in which you can contribute to growing OAXS 
or be involved in maturing the standard.

## Contributing to OAXS

The first way to contribute is by creating examples, implementations, or XPU specific OAXS toolchains or runtimes.

> If you are an XPU manufacturer and are looking to add your XPU to OAX, please see the sections [Creating an OAXS toolchain]() and [Creating an OAXS runtime]() below.

There are many places to get started:

* If you are building example pipelines that utilize OAXS that you would like to share with the larger community, please start with the [Using OAXS]() documentation.
* If you are looking whether a specific XPU supports OAXS, please see our [OAXS contributed impelementations](). 
* Ir you are contributing a novel XPU, please see [Creating an OAXS toolchain]() and [Creating an OAXS runtime]() below, and the templates in the linked repositories. 
* If you are looking to validate and test your implementation, please see our [OAXS testing]() documentation. This section also includes a description of how to formally [add your XPU to the OAXS ecosystem]().


## The OAXS steering group

OAX Standard was started by [Network Optix]() as an attempt to solve hardware / XPU incompatibility 
which was slowing market adoption of novel hardware breakthroughs. Thus, from its inception, OAXS has not been tied to 
individual hardware manufacturers: it is a user driven initiative trying to make adoption of *any* new accelerator 
designs into actual meaningful business applications as easy as possible.

The following people are core to the development of OAXS:

* ...

## Active users
Currently, OAX is used by the following companies and developer to create their edge AI/ML pipelines:

* Network Optix, Inc
* ...