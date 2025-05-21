#!/bin/bash

echo "Building aceOS using Docker..."

# Build the Docker image
docker-compose build

# Run the build process in the Docker container
docker-compose run --rm aceos-dev make clean all

# Run QEMU to test the OS
if [[ "$OSTYPE" == "darwin"* ]]; then
  # For macOS, we need to handle display differently
  docker run -it --rm -v $(pwd):/aceos aceos-aceos-dev qemu-system-i386 -fda /aceos/os_image.img -display cocoa
else
  # For Linux
  docker run -it --rm -v $(pwd):/aceos -e DISPLAY=$DISPLAY -v /tmp/.X11-unix:/tmp/.X11-unix aceos-aceos-dev qemu-system-i386 -fda /aceos/os_image.img
fi

echo "Build process complete!"
echo "If you want to exit QEMU, press Ctrl+Alt+2, then type 'quit' and press Enter" 