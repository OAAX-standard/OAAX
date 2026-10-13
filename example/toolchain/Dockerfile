FROM ubuntu:22.04

WORKDIR /app

# Install whatever your conversion tools need here, e.g.:
# RUN apt-get update -qq && apt-get install -y --no-install-recommends <packages>

COPY src/ /app/src/

# Entrypoint contract: <input-path> <output-directory> (see docs/Specification.md)
ENTRYPOINT ["bash", "/app/src/run-toolchain.sh"]
