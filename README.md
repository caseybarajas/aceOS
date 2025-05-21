# CaseyOS

A bootable operating system featuring a 16-bit bootloader that launches a 32-bit kernel, designed for educational purposes.

## Project Overview

CaseyOS is a basic operating system with:
- A 16-bit bootloader written in assembly
- A simple C kernel that displays text on screen
- Docker-based build system for cross-platform development

## Requirements

- Docker and Docker Compose
- On Windows: X server like VcXsrv for display forwarding (optional)

## Getting Started

### On Windows:

1. Install Docker Desktop for Windows
2. (Optional) Install VcXsrv and start it with "Multiple Windows", "Start no client", and "Disable access control" options
3. Run the build script:
   ```
   .\build-and-run.bat
   ```

### On Linux/macOS:

1. Install Docker and Docker Compose
2. Make the build script executable:
   ```
   chmod +x build-and-run.sh
   ```
3. Run the build script:
   ```
   ./build-and-run.sh
   ```

## Project Structure

- `boot.asm`: Bootloader that loads and jumps to the kernel
- `kernel.c`: Simple C kernel that displays text on screen
- `Makefile`: Build instructions for the OS
- `Dockerfile` and `docker-compose.yml`: Docker configuration for build environment

## How It Works

1. The bootloader is loaded at address 0x7C00 by the BIOS
2. The bootloader displays a welcome message
3. The bootloader loads the kernel from disk into memory at address 0x10000
4. The kernel initializes and displays its own message on screen

## Exiting QEMU

To exit QEMU:
1. Press Ctrl+Alt+2 to switch to the QEMU monitor
2. Type `quit` and press Enter

## Next Steps for Development

Future enhancements could include:
- GDT and IDT initialization
- Basic interrupt handling
- Simple keyboard driver
- Memory management
- A basic shell

## License

This project is open source and available for educational purposes. 