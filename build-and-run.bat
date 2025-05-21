@echo off
echo Building CaseyOS using Docker...

REM Build the Docker image
docker-compose build

REM First run just the compilation steps (not the image creation)
docker-compose run --rm caseyos-dev bash -c "make clean && nasm -f bin -o boot.bin boot/boot.asm && make kernel.bin"

REM Check if the compilation was successful
if %ERRORLEVEL% neq 0 (
    echo Compilation failed! Check the errors above.
    exit /b %ERRORLEVEL%
)

REM Create the disk image on the host system
echo Creating OS disk image locally...
type NUL > os_image.img
fsutil file seteof os_image.img 1474560

REM Run the Docker container again to finish building the image
docker-compose run --rm caseyos-dev bash -c "dd if=boot.bin of=os_image.img conv=notrunc && dd if=kernel.bin of=os_image.img seek=1 conv=notrunc"

REM Check if the image creation was successful
if %ERRORLEVEL% neq 0 (
    echo Image creation failed! Check the errors above.
    exit /b %ERRORLEVEL%
)

echo Build successful! Running QEMU...

REM Wait a moment to ensure any file handles are properly closed
timeout /t 2 /nobreak > nul

REM Run QEMU with debugging enabled in Docker with curses display 
REM Use read-only flag since we don't need to write to the disk image during emulation
docker-compose run --rm caseyos-dev qemu-system-i386 -display curses -drive file=os_image.img,format=raw,index=0,if=floppy,readonly=on -no-reboot