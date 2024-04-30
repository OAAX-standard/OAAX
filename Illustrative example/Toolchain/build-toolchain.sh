set -e

cd "$(dirname "$0")"

docker build -t oax-toolchain:latest .
mkdir build || true
docker save -o ./build/oax-toolchain-latest.tar oax-toolchain:latest