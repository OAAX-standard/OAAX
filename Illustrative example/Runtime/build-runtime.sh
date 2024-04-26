set -e

cd "$(dirname "$0")"

rm -rf build || true
mkdir build
cd build
cmake ..
make

echo "Build complete. The runtime library is located at '$(pwd)':"
ls -l *.so