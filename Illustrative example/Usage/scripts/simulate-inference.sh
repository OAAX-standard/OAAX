set -e

cd "$(dirname "$0")"
cd ..

rm -rf build || true
mkdir build
cd build
cmake ..
make

echo "Build complete."
echo "Simulating inference using the runtime library:"
cd ..
./build/main ./artifacts/libRuntimeLibrary.so ./artifacts/build/model.oaax