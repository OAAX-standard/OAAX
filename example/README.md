# Illustrative Example

A minimal, runnable example of the three OAAX components. It does no real inference; it shows the contracts a
**conversion toolchain**, a **runtime** and a **host application** must follow.

> Requires Linux (x86_64 or aarch64), `docker`, `cmake` and a C compiler.

```bash
bash run-example.sh
```

This builds everything into `example/build/`, converts `host/model.onnx`, and runs the host against the runtime.

## Layout

| Directory | What it is |
| --- | --- |
| [`toolchain/`](toolchain) | Docker image whose entrypoint takes `<input-path> <output-directory>` and writes the converted model (`model.oaax`) plus a conversion log (`logs.json`). |
| [`runtime/`](runtime) | Shared library implementing [`include/interface.h`](../include/interface.h): `runtime_init`, `runtime_load_models`, `runtime_enqueue_input`, `runtime_retrieve_output`, `runtime_cleanup` and the getters. |
| [`host/`](host) | Application that `dlopen`s a runtime, loads a model, sends 10 requests and checks each output's `Tensors.id` matches a request. |

Each part can also be built on its own:

```bash
bash toolchain/build-toolchain.sh   # builds the image and saves toolchain/build/oaax-toolchain-latest.tar
bash runtime/build-runtime.sh       # builds runtime/build/libRuntimeLibrary.so
```

## Writing your own

- **Toolchain**: see the [toolchain specification](https://oaax.org/Specification/#oaax-toolchain-specification).
  Start from [`toolchain/Dockerfile`](toolchain/Dockerfile) and [`run-toolchain.sh`](toolchain/src/run-toolchain.sh).
- **Runtime**: implement every function in [`include/interface.h`](../include/interface.h); the header documents what the
  caller and the runtime are each responsible for (memory ownership, threading, timeouts).
  [`runtime/src/library.c`](runtime/src/library.c) is a compact reference for the call flow.
- **Host**: [`host/main.c`](host/main.c) shows the full lifecycle, including who frees what.
