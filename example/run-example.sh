#!/usr/bin/env bash
# End-to-end demo: build the toolchain and runtime, convert a model, run it from a host app.
# Requires: docker, cmake, a C compiler.
set -euo pipefail
cd "$(dirname "$0")"

out="$PWD/build"
rm -rf "$out"
mkdir -p "$out/conversion"

echo "==> Building the toolchain image"
docker build -q -t oaax-toolchain:latest toolchain

echo "==> Converting the model"
# --user: the container must be able to write into the (host-owned) mounted directory
docker run --rm --user "$(id -u):$(id -g)" \
    -v "$PWD/host/model.onnx:/app/run/model.onnx:ro" \
    -v "$out/conversion:/app/output" \
    oaax-toolchain:latest /app/run/model.onnx /app/output
ls "$out/conversion"

echo "==> Building the runtime"
cmake -S runtime -B "$out/runtime" >/dev/null
cmake --build "$out/runtime"

echo "==> Building the host"
cmake -S host -B "$out/host" >/dev/null
cmake --build "$out/host"

echo "==> Running the host against the runtime"
"$out/host/host" "$out/runtime/libRuntimeLibrary.so" "$out/conversion/model.oaax"
