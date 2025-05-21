# makefile for caseyos

# tools
NASM = nasm
GCC = gcc
LD = ld
OBJCOPY = objcopy

# nasm flags for bootloader (output raw binary)
NASMFLAGS_BOOT = -f bin -o boot.bin

# gcc flags for kernel - we're targeting flat binary for simplicity
CFLAGS = -ffreestanding -nostdlib -m32 -fno-pie -fno-stack-protector -c -o kernel.o
LDFLAGS_KERNEL = -Ttext 0x10000 --oformat binary -m elf_i386 -o kernel.bin

# files
BOOT_SRC = boot.asm
KERNEL_C_SRC = kernel.c
KERNEL_OBJ = kernel.o
KERNEL_BIN = kernel.bin
OS_IMAGE = os_image.img

# default target
all: $(OS_IMAGE)

# build the bootloader
boot.bin: $(BOOT_SRC)
	$(NASM) $(NASMFLAGS_BOOT) $(BOOT_SRC)

# build the c kernel object file
$(KERNEL_OBJ): $(KERNEL_C_SRC)
	$(GCC) $(CFLAGS) $(KERNEL_C_SRC)

# link the c kernel object to a flat binary
$(KERNEL_BIN): $(KERNEL_OBJ)
	$(LD) $(LDFLAGS_KERNEL) $(KERNEL_OBJ)

# create the final os image (bootloader + kernel)
$(OS_IMAGE): boot.bin $(KERNEL_BIN)
	@echo "Creating OS disk image..."
	# Create an empty 1.44MB floppy disk image (512 bytes/sector × 2880 sectors = 1.44MB)
	dd if=/dev/zero of=$(OS_IMAGE) bs=512 count=2880
	# Write bootloader to sector 1 (the first sector, 512 bytes) without overwriting the entire image
	dd if=boot.bin of=$(OS_IMAGE) conv=notrunc
	# Write kernel starting at sector 2 (seek=1 means skip 1 sector, which is 512 bytes)
	# The bootloader will load this kernel from sector 2 into memory
	dd if=$(KERNEL_BIN) of=$(OS_IMAGE) seek=1 conv=notrunc

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
