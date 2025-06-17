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
if %ERRORLEVEL% neq 0 (
    echo Failed to build Docker image!
    exit /b 1
)

REM Run make clean to ensure a fresh build
echo Cleaning previous build...
docker-compose run --rm aceos-dev make clean

REM Build the OS
echo Building aceOS...
docker-compose run --rm aceos-dev make
if %ERRORLEVEL% neq 0 (
    echo Build failed!
    exit /b 1
)

REM Check if build was successful
if not exist "os_image.img" (
    echo Build failed! os_image.img not found.
    exit /b 1
)

echo Build successful! Running aceOS in QEMU...

REM Check if local QEMU is available first
where qemu-system-i386 >nul 2>nul
if %ERRORLEVEL% equ 0 (
    echo Using local QEMU installation...
    qemu-system-i386 -fda os_image.img -serial stdio
    goto end
)

REM If we get here, local QEMU isn't available, try to run with Docker
echo Local QEMU not found, using Docker...
echo Note: For graphical display on Windows, make sure X server (VcXsrv) is running
echo You can download VcXsrv from: https://sourceforge.net/projects/vcxsrv/

REM Try to find if VcXsrv is running
tasklist | find "vcxsrv" >nul 2>nul
set VCXSRV_RUNNING=%ERRORLEVEL%

if %VCXSRV_RUNNING% equ 0 (
    echo VcXsrv detected, using graphical display...
    set DISPLAY=127.0.0.1:0.0
    docker-compose run --rm -e DISPLAY=%DISPLAY% aceos-dev qemu-system-i386 -fda /aceos/os_image.img -serial stdio
) else (
    echo VcXsrv not detected, running in text mode...
    echo Note: Serial output will be displayed in this console
    docker-compose run --rm aceos-dev qemu-system-i386 -fda /aceos/os_image.img -nographic -serial stdio
)

:end
echo aceOS execution completed!