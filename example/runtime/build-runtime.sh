#!/usr/bin/env bash
# Builds the example runtime into runtime/build/libRuntimeLibrary.so
set -euo pipefail
cd "$(dirname "$0")"

cmake -S . -B build
cmake --build build

echo "Build complete: $(pwd)/build/libRuntimeLibrary.so"
