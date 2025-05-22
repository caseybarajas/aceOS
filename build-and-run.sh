#!/bin/bash

# Build and run script for aceOS using Docker

# Check if Docker is installed
if ! command -v docker &> /dev/null; then
    echo "Docker is not installed. Please install Docker first."
    exit 1
fi

# Check if docker-compose is installed
if ! command -v docker-compose &> /dev/null; then
    echo "Docker Compose is not installed. Please install Docker Compose first."
    exit 1
fi

# Build the Docker image if needed
echo "Building Docker development environment..."
docker-compose build

# Run make clean to ensure a fresh build
echo "Cleaning previous build..."
docker-compose run --rm aceos-dev make clean

# Build the OS
echo "Building aceOS..."
docker-compose run --rm aceos-dev make

# Run the OS in QEMU with graphical display
echo "Running aceOS in QEMU..."

# If on macOS, handle display differently
if [[ "$OSTYPE" == "darwin"* ]]; then
  docker run --rm -v "$(pwd)":/aceos caseyos-aceos-dev qemu-system-i386 -fda /aceos/os_image.img
else
  # For Linux
  xhost +local:docker
  docker run --rm -v "$(pwd)":/aceos -e DISPLAY=$DISPLAY -v /tmp/.X11-unix:/tmp/.X11-unix caseyos-aceos-dev qemu-system-i386 -fda /aceos/os_image.img
fi

echo "Done!" 