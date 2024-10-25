# Illustrative Example

This illustrative example aims to provide a starting point for all parties involved in the OAAX initiative:
contributors and users.
It details the essential components that a runtime, and a conversion toolchain should have. 
However, this code doesn't run real-world models, it's used for illustrative purposes only.

> This example is meant to be run on a Ubuntu 20.04+ machine on a x86_64 architecture.

## Overview

The example is split into two main parts:

- The first part provides a simple implementation of an OAAX runtime and an OAAX conversion toolchain.
  It mainly sets the common ground for the OAAX initiative and provides a skeleton implementation for the OAAX runtime
  and conversion toolchain.
- The second part shows how to use any prebuilt OAAX toolchain and runtime to convert a simple model and then run it.

## Part 1: Reference Implementation

### OAAX Conversion Toolchain

The conversion toolchain is expected to convert a platform-agnostic model to a platform-specific model format.
Hence, the toolchain, which is in practice a docker image, is expected to have an entrypoint that takes two parameters:

1. The path to the platform-agnostic model.
2. The path to the output directory where the platform-specific model will be saved, along with a logs file that
   contains the conversion process logs.

An example of each component is provided in the [Toolchain](Toolchain) directory:

- [Dockerfile](Toolchain/Dockerfile): provides a simple Dockerfile that installs the necessary dependencies to run the
  conversion toolchain.
- [Entrypoint](Toolchain/src/run-toolchain.sh): provides a simple entrypoint script that takes the two parameters and
  runs the conversion process.
- [Logs](Toolchain/src/dummy_logs.json): provides an example of the logs file that should be generated by the conversion
  toolchain.

To build the example docker image, run the following command (Note that you need to have docker installed):

```bash 
bash Toolchain/build-toolchain.sh
```

This will build the docker image, and save it in the `Toolchain/build/` directory.

### OAAX Runtime

[Runtime](Runtime/src/library.c) provides a simple implementation of the OAAX runtime.
The runtime is expected to "load" the platform-specific model and run it asynchronously.

The full interface that the OAAX runtime is expected to adhere to is provided is
this [header](Runtime/include/interface.h) file.

To build the runtime, run the following command (cmake and a C compiler are required):

```bash
bash Runtime/build-runtime.sh
```

## Part 2: Usage Example

This part shows how to use the prebuilt OAAX toolchain and runtime to demonstrate the conversion of a simple model and
running it.

### Convert a model

Before starting to convert a model, make sure that the example toolchain docker image and an ONNX file are present in
the `Usage/artifacts/` directory.

To convert a model, run the following command:

```bash
bash Usage/scripts/simulate-conversion.sh
```

This command will load the docker image that's saved in the ".tar" file:

```bash
docker load -i "./artifacts/oaax-toolchain-latest.tar"
```

and run the conversion process on the ONNX file:

```bash
docker run -v "./artifacts:/app/run" oaax-toolchain "/app/run/model.onnx" "/app/run/build"
```

The resulting model file will be saved in the `Usage/artifacts/build/` directory, along with a logs file that contains
the conversion process logs.

### Run the model

After converting the model (available at: `Usage/artifacts/build/`),
the next step is to run it using the example OAAX runtime.
To simulate the inference process, run the following command:

```bash
bash Usage/scripts/simulate-inference.sh
```

This will build the example code available in [main.c](Usage/main.c):

```bash
cd build
cmake ..
make
```

and run the pre-converted model (`model.oaax`) using the pre-built runtime library (`libRuntimeLibrary.so`):

```bash
./build/main ./artifacts/libRuntimeLibrary.so ./artifacts/build/model.oaax
```
