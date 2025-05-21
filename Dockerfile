FROM ubuntu:22.04

# set noninteractive installation
ENV DEBIAN_FRONTEND=noninteractive

# install dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    gcc \
    nasm \
    make \
    qemu-system-x86 \
    gdb \
    xorriso \
    grub-pc-bin \
    grub-common \
    && rm -rf /var/lib/apt/lists/*

# set working directory
WORKDIR /caseyos

# volume for code
VOLUME ["/caseyos"]

# default command
CMD ["/bin/bash"] 