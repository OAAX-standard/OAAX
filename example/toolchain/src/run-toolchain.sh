#!/usr/bin/env bash
# Usage: run-toolchain.sh <input-path> <output-directory>
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"

if [ "$#" -ne 2 ]; then
    echo "Usage: run-toolchain.sh <input-path> <output-directory>" >&2
    exit 1
fi

model_path=$1
output_directory=$2

mkdir -p "$output_directory"

if [ ! -f "$model_path" ]; then
    echo "File not found: $model_path" >&2
    exit 1
fi

# Simulate conversion time
sleep 1

# A real toolchain would convert the model here. We just copy it under a different name.
cp "$model_path" "$output_directory/model.oaax"
cp "$SCRIPT_DIR/dummy_logs.json" "$output_directory/logs.json"
