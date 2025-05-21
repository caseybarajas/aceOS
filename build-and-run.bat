@echo off
echo Building CaseyOS using Docker...

REM Build the Docker image
docker-compose build

REM Run the build process in the Docker container
docker-compose run --rm caseyos-dev make clean all

REM Run QEMU with curses display (text-based UI) and debug output
docker run -it --rm -v %cd%:/caseyos caseyos-caseyos-dev qemu-system-i386 -fda /caseyos/os_image.img -display curses -serial stdio

echo Build process complete!
echo If you want to exit QEMU, press Ctrl+Alt+2, then type 'quit' and press Enter 