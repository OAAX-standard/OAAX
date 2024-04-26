set -e

cd "$(dirname "$0")"
cd ..

toolchain_path="./artifacts/oax-toolchain-latest.tar"
output_directory="./artifacts"
toolchain_container_name="oax-toolchain-container"

echo "Loading the toolchain image into Docker..."
docker load -i "$toolchain_path"

echo "Running the toolchain on the model..."
docker stop $toolchain_container_name 2&> /dev/null || true
docker rm $toolchain_container_name 2&> /dev/null || true
docker run --name $toolchain_container_name -v "$output_directory:/app/run" oax-toolchain "/app/run/model.onnx" "/app/run/build"

echo "Toolchain run complete."
echo "The toolchain produced the following files in the output directory:"
ls "$output_directory/build"