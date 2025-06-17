@echo off
echo Building aceOS using Docker...

REM Check if Docker is installed
where docker >nul 2>nul
if %ERRORLEVEL% neq 0 (
    echo Docker is not installed. Please install Docker first.
    exit /b 1
)

REM Check if docker-compose is installed
where docker-compose >nul 2>nul
if %ERRORLEVEL% neq 0 (
    echo Docker Compose is not installed. Please install Docker Compose first.
    exit /b 1
)

REM Build the Docker image if needed
echo Building Docker development environment...
docker-compose build

REM Run make clean to ensure a fresh build
echo Cleaning previous build...
docker-compose run --rm aceos-dev make clean

REM Build the OS
echo Building aceOS...
docker-compose run --rm aceos-dev make

echo Running aceOS in QEMU...

REM Check if local QEMU is available
where qemu-system-i386 >nul 2>nul
if %ERRORLEVEL% equ 0 (
    echo Using local QEMU installation...
    qemu-system-i386 -fda os_image.img
    goto end
)

REM If we get here, local QEMU isn't available, try to run with Docker
echo Local QEMU not found, using Docker...
echo Note: For graphical display, make sure X server (VcXsrv) is running on Windows
echo You can download VcXsrv from: https://sourceforge.net/projects/vcxsrv/

REM Try to find if VcXsrv is running
tasklist | find "vcxsrv" >nul 2>nul
set VCXSRV_RUNNING=%ERRORLEVEL%

if %VCXSRV_RUNNING% equ 0 (
    echo VcXsrv detected, using graphical display...
    set DISPLAY=127.0.0.1:0.0
    docker run --rm -e DISPLAY=%DISPLAY% -v %cd%:/aceos aceOS-aceos-dev qemu-system-i386 -fda /aceos/os_image.img
) else (
    echo VcXsrv not detected, using standard display...
    docker run --rm -v %cd%:/aceos aceOS-aceos-dev qemu-system-i386 -fda /aceos/os_image.img
)

:end
echo Done!