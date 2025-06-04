# CaseyOS

A bootable operating system featuring a 16-bit bootloader that launches a 32-bit kernel, designed for educational purposes.

## Features

### Core System
- **16-bit Bootloader**: Loads the kernel from disk into memory
- **32-bit Protected Mode Kernel**: Full kernel with interrupt handling
- **Interrupt System**: IDT setup with keyboard and serial interrupts
- **Serial Debug Output**: COM1 port debugging support
- **VGA Text Mode**: 80x25 character display

### Comprehensive Filesystem
- **In-Memory Filesystem**: Complete hierarchical file system
- **Directory Support**: Nested directories with full path navigation
- **File Operations**: Create, read, write, copy, move, delete
- **Current Working Directory**: Shell maintains directory context
- **Search Functionality**: Find files by name patterns
- **Tree View**: Visual directory structure display
- **File Metadata**: Size, type, and creation information

### Shell Commands
- `help` - Show available commands
- `clear` - Clear the screen
- `version` - Show OS version
- `echo <text>` - Echo text to screen
- `debug` - Send debug info to serial port

#### Directory Navigation
- `pwd` - Show current working directory
- `cd <path>` - Change directory (supports `.` and `..`)
- `ls [path]` - List directory contents

#### File Management
- `mkdir <path>` - Create directories
- `touch <path>` - Create empty files
- `cat <path>` - Display file contents
- `write <path> <content>` - Write content to files
- `cp <source> <destination>` - Copy files
- `mv <source> <destination>` - Move/rename files
- `rm <path>` - Remove files or directories

#### Filesystem Tools
- `find <pattern>` - Search for files by name pattern
- `tree [path]` - Display directory tree structure
- `stat <path>` - Show detailed file information
- `fsinfo` - Display filesystem statistics

## Building and Running

### Using Docker (Recommended for Windows)
```bash
docker-compose up -d
docker-compose exec aceos-dev make clean
docker-compose exec aceos-dev make
docker-compose exec aceos-dev qemu-system-i386 -fda os_image.img
```

### Using Build Scripts
```bash
# Linux/Mac
./build-and-run.sh

# Windows
./build-and-run.bat
```

### Manual Build
```bash
make clean
make
qemu-system-i386 -fda os_image.img
```

## Filesystem Specifications

- **Maximum Directories**: 32
- **Maximum Files**: 128
- **Files per Directory**: 64
- **Filename Length**: 32 characters
- **Path Length**: 256 characters
- **Storage**: Dynamic memory allocation for file content

## Architecture

```
CaseyOS/
├── boot/           # 16-bit bootloader assembly
├── kernel/         # 32-bit kernel C code
├── drivers/        # Device drivers (keyboard, serial, filesystem)
├── include/        # Header files
└── src/libc/      # C standard library implementation
```

## Example Usage

```
aceOS> help
aceOS> mkdir /home
aceOS> cd /home
aceOS> touch test.txt
aceOS> write test.txt Hello from CaseyOS!
aceOS> cat test.txt
aceOS> cp test.txt backup.txt
aceOS> ls
aceOS> tree /
aceOS> find test
aceOS> stat test.txt
```

## Development

Built with:
- **NASM** for assembly code
- **GCC** for C code compilation
- **Docker** for consistent build environment
- **QEMU** for testing and emulation

