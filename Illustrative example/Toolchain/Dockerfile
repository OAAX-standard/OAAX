ARG DEBIAN_FRONTEND=noninteractive
FROM ubuntu:22.04

WORKDIR /app

# You can for example install any dependencies the toolchain needs here
RUN #apt-get update -qq -y --fix-missing

COPY src/ /app/src/

ENTRYPOINT ["bash", "/app/src/run-toolchain.sh"]