# aceOS

A bootable operating system featuring a 16-bit bootloader that launches a 32-bit kernel, designed for educational purposes.

## Project Overview

aceOS is a basic operating system with:
- A 16-bit bootloader written in assembly
- A simple C kernel that displays text on screen
- Custom C standard library implementation
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

- `boot/`: Contains bootloader code
- `kernel/`: Contains kernel implementation
- `drivers/`: Hardware drivers for keyboard, etc.
- `include/`: Header files
- `include/libc/`: C standard library headers
- `src/libc/`: C standard library implementation
- `Makefile`: Build instructions for the OS
- `Dockerfile` and `docker-compose.yml`: Docker configuration for build environment

## C Standard Library

aceOS includes a custom implementation of the C standard library with:
- String functions (strlen, strcpy, etc.)
- Memory functions (memcpy, memset, etc.)
- Standard I/O (printf, putchar, etc.)
- Memory allocation (malloc, free, etc.)

See `src/libc/README.md` for more details.

## How It Works

1. The bootloader is loaded at address 0x7C00 by the BIOS
2. The bootloader displays a welcome message
3. The bootloader loads the kernel from disk into memory at address 0x10000
4. The kernel initializes hardware and the C standard library
5. The kernel displays a simple shell interface

## Exiting QEMU

To exit QEMU:
1. Press Ctrl+Alt+2 to switch to the QEMU monitor
2. Type `quit` and press Enter

## Next Steps for Development

Future enhancements could include:
- Extended C standard library support
- File system implementation
- Process management
- Memory protection
- A more advanced shell

## License

This project is open source and available for educational purposes. 