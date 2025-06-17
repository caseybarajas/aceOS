# makefile for aceOS

# tools
NASM = nasm
GCC = gcc
LD = ld
OBJCOPY = objcopy

# directories
INCLUDE_DIR = include
BOOT_DIR = boot
KERNEL_DIR = kernel
DRIVERS_DIR = drivers
LIBC_DIR = src/libc

# nasm flags for bootloader (output raw binary)
NASMFLAGS_BOOT = -f bin -o boot.bin
NASMFLAGS_INT = -f elf32 -o kernel/interrupt.o

# gcc flags for kernel - we're targeting flat binary for simplicity
CFLAGS = -ffreestanding -nostdlib -m32 -fno-pie -fno-stack-protector -fno-asynchronous-unwind-tables -c -I$(INCLUDE_DIR) -D__GNUC__
LDFLAGS_KERNEL = -Ttext 0x10000 --oformat binary -m elf_i386 -o kernel.bin

# files
BOOT_SRC = $(BOOT_DIR)/boot.asm
KERNEL_SRCS = $(KERNEL_DIR)/kernel.c $(KERNEL_DIR)/idt.c $(KERNEL_DIR)/isr.c $(KERNEL_DIR)/pic.c $(KERNEL_DIR)/pmm.c $(KERNEL_DIR)/vmm.c $(KERNEL_DIR)/heap.c $(KERNEL_DIR)/memory_utils.c $(KERNEL_DIR)/process.c $(KERNEL_DIR)/scheduler.c
DRIVER_SRCS = $(DRIVERS_DIR)/keyboard.c $(DRIVERS_DIR)/serial.c $(DRIVERS_DIR)/fs.c $(DRIVERS_DIR)/timer.c $(DRIVERS_DIR)/disk.c
INT_ASM_SRC = $(KERNEL_DIR)/interrupt.asm
PAGING_ASM_SRC = $(KERNEL_DIR)/paging.asm
LIBC_SRCS = $(LIBC_DIR)/string.c $(LIBC_DIR)/stdio.c $(LIBC_DIR)/stdlib.c $(LIBC_DIR)/libc.c

# objects
KERNEL_OBJS = $(KERNEL_SRCS:.c=.o)
DRIVER_OBJS = $(DRIVER_SRCS:.c=.o)
INT_OBJ = $(KERNEL_DIR)/interrupt.o
PAGING_OBJ = $(KERNEL_DIR)/paging.o
LIBC_OBJS = $(LIBC_SRCS:.c=.o)
ALL_OBJS = $(KERNEL_OBJS) $(DRIVER_OBJS) $(INT_OBJ) $(PAGING_OBJ) $(LIBC_OBJS)

# output files
KERNEL_BIN = kernel.bin
OS_IMAGE = os_image.img

# default target
all: $(OS_IMAGE)

# build the bootloader
boot.bin: $(BOOT_SRC)
	$(NASM) $(NASMFLAGS_BOOT) $(BOOT_SRC)

# build the interrupt assembly file
$(INT_OBJ): $(INT_ASM_SRC)
	$(NASM) $(NASMFLAGS_INT) $(INT_ASM_SRC)

# build the paging assembly file
$(PAGING_OBJ): $(PAGING_ASM_SRC)
	$(NASM) -f elf32 -o $(PAGING_OBJ) $(PAGING_ASM_SRC)

# build the c kernel and driver object files
%.o: %.c
	$(GCC) $(CFLAGS) -o $@ $<

# link the objects to a flat binary
$(KERNEL_BIN): $(ALL_OBJS)
	$(LD) $(LDFLAGS_KERNEL) $(ALL_OBJS)

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
	rm -f boot.bin $(ALL_OBJS) $(KERNEL_BIN) $(OS_IMAGE) *.o

# rule to run with qemu
run:
	@if [ -f /.dockerenv ]; then \
		qemu-system-i386 -fda $(OS_IMAGE); \
	else \
		qemu-system-i386 -fda $(OS_IMAGE); \
	fi

# for running with no graphics (serial output only)
run-nographic:
	qemu-system-i386 -nographic -fda $(OS_IMAGE)
