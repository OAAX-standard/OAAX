set -e

cd "$(dirname "$0")"

docker build -t oaax-toolchain:latest .
mkdir build || true
docker save -o ./build/oaax-toolchain-latest.tar oaax-toolchain:latest