@echo off
echo Building CaseyOS using Docker...

REM Build the Docker image
docker-compose build

REM Run the build process in the Docker container
docker-compose run --rm caseyos-dev make clean all

REM Check if the build was successful
if %ERRORLEVEL% neq 0 (
    echo Build failed! Check the errors above.
    exit /b %ERRORLEVEL%
)

echo Build successful! Running QEMU...

REM Run QEMU with debugging enabled
qemu-system-i386 -drive file=os_image.img,format=raw,index=0,if=floppy -no-reboot