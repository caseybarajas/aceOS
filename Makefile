# makefile for caseyos

# tools
NASM = nasm
GCC = gcc
LD = ld
OBJCOPY = objcopy

# nasm flags for bootloader (output raw binary)
NASMFLAGS_BOOT = -f bin -o boot.bin

# gcc flags for kernel (16-bit, freestanding, no standard libs, position independent)
# note: compiling c for 16-bit real mode to be called from a bootloader is tricky.
# you need a 16-bit target compiler or a cross-compiler setup.
# this makefile assumes you have a gcc cross-compiler for i386-elf or similar,
# or that your native gcc can output 16-bit code (less common for modern systems).
# a more common approach for early c kernels is to switch to 32-bit protected mode first.
# for simplicity, this makefile might need adjustment based on your c toolchain.
#
# if you are using a standard 32/64-bit gcc, it won't produce 16-bit real mode code directly.
# you might need something like `i686-elf-gcc` and specific flags.
#
# for a *very* simple c kernel that does almost nothing (like the stub above),
# you might get away with compiling it as a flat binary and loading it,
# but memory addressing and calling conventions are critical.

# for this example, let's assume we are creating a flat binary for the kernel.
# this is a simplification and might not work for more complex c code without a proper toolchain.
CFLAGS = -m16 -ffreestanding -nostdlib -O2 -Wall -Wextra -c -o kernel.o
LDFLAGS_KERNEL = -Ttext 0x1000 --oformat binary -o kernel.bin # link kernel to run at 0x1000

# files
BOOT_SRC = boot.asm
KERNEL_C_SRC = kernel.c
KERNEL_OBJ = kernel.o
KERNEL_BIN = kernel.bin # the c kernel compiled to a binary
OS_IMAGE = os_image.img

# default target
all: $(OS_IMAGE)

# build the bootloader
boot.bin: $(BOOT_SRC)
	$(NASM) $(NASMFLAGS_BOOT) $(BOOT_SRC)

# build the c kernel object file
# this step is highly dependent on your gcc setup for 16-bit real mode.
# you might need a specific target like i386-elf-gcc.
# using a standard gcc might produce incompatible code.
$(KERNEL_OBJ): $(KERNEL_C_SRC)
	@echo "attempting to compile kernel.c for 16-bit real mode."
	@echo "this step is highly sensitive to your gcc toolchain."
	@echo "you may need an i386-elf cross-compiler and adjust cflags."
	$(GCC) $(CFLAGS) $(KERNEL_C_SRC)

# link the c kernel object to a flat binary
# this also depends on having a linker that can produce a 16-bit flat binary.
$(KERNEL_BIN): $(KERNEL_OBJ)
	$(LD) $(LDFLAGS_KERNEL) $(KERNEL_OBJ)

# create the final os image (bootloader + kernel)
# this simple example just creates a 1.44mb floppy disk image
# with the bootloader at the start. for now, it doesn't append the kernel.
# to properly load the kernel, the bootloader would need code to:
# 1. detect kernel size and location (e.g., from a known sector on the disk)
# 2. load the kernel from disk into memory (e.g., at address 0x10000)
# 3. jump to the kernel's entry point.
#
# this makefile currently only builds boot.bin.
# the kernel compilation is illustrative and needs a proper 16-bit c toolchain.
# for a first step, you can focus on just the bootloader.
$(OS_IMAGE): boot.bin # $(KERNEL_BIN) # add kernel_bin if you have a working c kernel compilation
	@echo "creating floppy image..."
	dd if=/dev/zero of=$(OS_IMAGE) bs=512 count=2880 # create a 1.44mb empty floppy image
	dd if=boot.bin of=$(OS_IMAGE) conv=notrunc      # copy bootloader to the image
	# if kernel_bin is successfully built and you want to add it:
	# dd if=$(KERNEL_BIN) of=$(OS_IMAGE) seek=1 conv=notrunc # example: copy kernel to sector 1

# clean up build files
clean:
	rm -f boot.bin $(KERNEL_OBJ) $(KERNEL_BIN) $(OS_IMAGE) *.o

# rule to run with qemu
run:
	@if [ -f /.dockerenv ]; then \
		qemu-system-i386 -display curses -fda $(OS_IMAGE); \
	else \
		qemu-system-i386 -fda $(OS_IMAGE); \
	fi

# for running with no graphics (serial output only)
run-nographic:
	qemu-system-i386 -nographic -fda $(OS_IMAGE)

# note on kernel loading:
# the current boot.asm does not load kernel.bin.
# to make it load the kernel, you would:
# 1. modify boot.asm to read sectors from the floppy (e.g., starting at sector 1)
#    into a memory location (e.g., 0x1000:0000 or linear address 0x10000).
# 2. after loading, jump to the kernel's entry point (e.g., `call 0x1000:0000`).
# 3. the makefile would need to concatenate boot.bin and kernel.bin correctly
#    into os_image.img, or place kernel.bin at a known sector.
#
# for now, `make run` will just run the bootloader.
