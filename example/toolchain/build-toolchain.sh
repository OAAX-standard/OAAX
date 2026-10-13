#!/usr/bin/env bash
# Builds the example toolchain image and saves it as toolchain/build/oaax-toolchain-latest.tar
set -euo pipefail
cd "$(dirname "$0")"

docker build -t oaax-toolchain:latest .
mkdir -p build
docker save -o ./build/oaax-toolchain-latest.tar oaax-toolchain:latest
