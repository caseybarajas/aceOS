#!/bin/bash

echo "Building CaseyOS using Docker..."

# Build the Docker image
docker-compose build

# Run the build process in the Docker container
docker-compose run --rm caseyos-dev bash -c "make clean && make all"

# Check if the build was successful
if [ $? -ne 0 ]; then
    echo "Build failed! Check the errors above."
    exit 1
fi

echo "Build successful! Running QEMU..."

# Run QEMU to test the OS
if [[ "$OSTYPE" == "darwin"* ]]; then
  # For macOS, we need to handle display differently
  docker-compose run --rm caseyos-dev qemu-system-i386 -fda os_image.img -display cocoa
else
  # For Linux
  docker-compose run --rm caseyos-dev qemu-system-i386 -display curses -fda os_image.img
fi

echo "If you want to exit QEMU, press Ctrl+Alt+2, then type 'quit' and press Enter" 