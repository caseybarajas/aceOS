# CaseyOS

A simple MS-DOS style operating system built from scratch for learning purposes.

## Project Overview

CaseyOS is a basic operating system with the following components:
- A 16-bit bootloader written in assembly (NASM)
- A minimal C kernel
- A build system using Make

## Development Environment

This project includes a Docker setup for development which provides all the necessary tools:
- NASM (assembler)
- GCC (C compiler)
- Make (build system)
- QEMU (emulator)

### Getting Started with Docker

1. Build and start the development container:
   ```
   docker-compose up -d
   ```

2. Access the container shell:
   ```
   docker exec -it caseyos-dev bash
   ```

3. Inside the container, you can build and run CaseyOS:
   ```
   make clean
   make
   make run
   ```

### Building Without Docker

If you prefer not to use Docker, you'll need to install the following tools:

#### Windows:
```powershell
# Install Chocolatey if not already installed
Set-ExecutionPolicy Bypass -Scope Process -Force
[System.Net.ServicePointManager]::SecurityProtocol = [System.Net.ServicePointManager]::SecurityProtocol -bor 3072
iex ((New-Object System.Net.WebClient).DownloadString('https://community.chocolatey.org/install.ps1'))

# Install required tools using Chocolatey
choco install -y nasm mingw make qemu
```

#### Build and Run
```
make clean
make
make run
```

## Project Structure

- `boot.asm`: Bootloader code
- `kernel.c`: C kernel stub
- `Makefile`: Build configuration
- `Notes/design-notes.md`: Detailed project documentation

## Current Status

Currently, the bootloader successfully:
1. Initializes segments
2. Clears the screen
3. Prints a welcome message
4. Halts the CPU

Next steps include loading the C kernel from the bootloader. 