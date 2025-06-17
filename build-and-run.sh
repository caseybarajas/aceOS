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

# Check if build was successful
if [ ! -f "os_image.img" ]; then
    echo "Build failed! os_image.img not found."
    exit 1
fi

echo "Build successful! Running aceOS in QEMU..."

# Run the OS in QEMU with graphical display
if [[ "$OSTYPE" == "darwin"* ]]; then
  # For macOS
  echo "Running on macOS..."
  docker-compose run --rm aceos-dev qemu-system-i386 -fda /aceos/os_image.img -serial stdio
else
  # For Linux
  echo "Running on Linux..."
  xhost +local:docker 2>/dev/null || echo "Warning: Could not configure X11 forwarding"
  docker-compose run --rm -e DISPLAY=$DISPLAY aceos-dev qemu-system-i386 -fda /aceos/os_image.img -serial stdio
fi

echo "aceOS execution completed!" 