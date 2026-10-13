set -e
cd "$(dirname "$0")"

# Usage: run-toolchain.sh <path-to-model> <output-directory>
if [ "$#" -ne 2 ]; then
    echo "Usage: run-toolchain <path-to-model> <output-directory>"
    exit 1
fi

model_path=$1
output_directory=$2

mkdir -p "$output_directory" || true

# Simulate conversion time by sleeping for 3 seconds
sleep 3

# Copy same model to output directory under a different name
cp "$model_path" "$output_directory/model.oaax"
# Copy the dummy logs file
cp ./dummy_logs.json "$output_directory/logs.json"