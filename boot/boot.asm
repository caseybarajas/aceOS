; boot.asm
; a very simple bootloader for aceos.
; this bootloader prints "bootloader loaded. welcome to aceos!" to the screen.
; it loads a c kernel and switches to 32-bit protected mode before jumping to it.

bits 16             ; we are in 16-bit real mode

ORG 0x7C00          ; bios loads our bootloader at this address

start:              ; start of bootloader code
    cli             ; disable interrupts initially
    xor ax, ax      ; zero ax
    mov ds, ax      ; set all segment registers to 0
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00  ; stack pointer grows downwards from 0x7c00
    sti             ; re-enable interrupts for BIOS services

    ; clear the screen
    mov ah, 0x00    ; bios video service: set video mode
    mov al, 0x03    ; text mode 80x25, 16 colors
    int 0x10

    ; disable hardware cursor
    call disable_cursor

    ; print the welcome message
    mov si, message ; point si to our message

load_kernel:
    ; print kernel loading message
    mov si, kernel_loading_msg
    call print_string
    
    ; reset disk system
    xor ah, ah      ; function 0 - reset disk
    xor dl, dl      ; drive 0 (floppy)
    int 0x13
    jc disk_error   ; if carry flag is set, error occurred
    
    ; load the kernel (from sector 2, right after bootloader)
    mov ah, 0x02    ; BIOS read sector function
    mov al, 60      ; number of sectors to read (30KB, should be enough for kernel)
    xor ch, ch      ; cylinder 0
    mov cl, 2       ; sector 2 (sectors are 1-based, sector 1 is bootloader)
    xor dh, dh      ; head 0
    xor dl, dl      ; drive 0 (floppy)
    
    ; set buffer where to load the kernel
    mov bx, 0x1000  ; load kernel to segment 0x1000
    mov es, bx
    xor bx, bx      ; offset 0 in segment - so address 0x10000

    int 0x13        ; call BIOS disk read
    jc disk_error   ; if carry flag is set, error occurred
    
    ; print kernel loaded message
    mov si, kernel_loaded_msg
    call print_string
    
    ; Wait a moment for message to be visible
    mov cx, 0xFFFF
delay_loop:
    loop delay_loop

    ; Now switch to protected mode
    cli             ; disable interrupts
    
    ; Load GDT register
    lgdt [gdt_descriptor]
    
    ; Set protected mode bit in CR0
    mov eax, cr0
    or eax, 0x1     ; set PE (Protection Enable) bit
    mov cr0, eax
    
    ; Far jump to clear the pipeline and load CS with 32-bit selector
    jmp CODE_SEG:protected_mode_entry

; GDT (Global Descriptor Table) for protected mode
gdt_start:
    ; Null descriptor (required)
    dq 0            ; 8 bytes of zeros

gdt_code:           ; CS should point to this descriptor
    dw 0xFFFF       ; limit (bits 0-15)
    dw 0            ; base (bits 0-15)
    db 0            ; base (bits 16-23)
    db 10011010b    ; access byte: present, ring 0, code segment, executable, direction 0, readable
    db 11001111b    ; flags + limit (bits 16-19): 4k granularity, 32-bit, limit bits 16-19
    db 0            ; base (bits 24-31)

gdt_data:           ; DS, SS, ES, FS, GS should point to this descriptor
    dw 0xFFFF       ; limit (bits 0-15)
    dw 0            ; base (bits 0-15)
    db 0            ; base (bits 16-23)
    db 10010010b    ; access byte: present, ring 0, data segment, direction 0, writable
    db 11001111b    ; flags + limit (bits 16-19): 4k granularity, 32-bit, limit bits 16-19
    db 0            ; base (bits 24-31)

gdt_end:            ; for calculating GDT size

gdt_descriptor:
    dw gdt_end - gdt_start - 1  ; size of GDT, always one less than true size
    dd gdt_start                ; address (32-bit) of GDT

; Define constants for GDT segment selectors
CODE_SEG equ gdt_code - gdt_start
DATA_SEG equ gdt_data - gdt_start

; 32-bit protected mode code
[bits 32]
protected_mode_entry:
    ; Set up data segment registers
    mov ax, DATA_SEG
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    
    ; Set up 32-bit stack at the top of free memory
    mov esp, 0x90000
    
    ; Jump to the kernel entry point (loaded at 0x10000)
    jmp 0x10000

[bits 16]
; print null-terminated string pointed to by si
print_string:
    pusha           ; save registers
print_string_loop:
    lodsb           ; load byte from [si] into al and increment si
    or al, al       ; check if al is 0 (end of string)
    jz print_string_done
    mov ah, 0x0E    ; BIOS teletype function
    int 0x10        ; call BIOS
    jmp print_string_loop
print_string_done:
    popa            ; restore registers
    ret

disk_error:
    mov si, disk_error_msg
    call print_string
    jmp halt_system

halt_system:
    cli             ; disable interrupts
    hlt             ; halt the processor
    jmp halt_system ; in case of interrupt, halt again

message:
    db "Bootloader loaded. Welcome to aceOS!", 13, 10, 0 ; with CR+LF

kernel_loading_msg:
    db "Loading kernel...", 13, 10, 0

kernel_loaded_msg:
    db "Kernel loaded successfully! Switching to protected mode...", 13, 10, 0

disk_error_msg:
    db "Disk read error!", 13, 10, 0

disable_cursor:
	pushf
	push eax
	push edx

	mov dx, 0x3D4
	mov al, 0xA	; low cursor shape register
	out dx, al

	inc dx
	mov al, 0x20	; bits 6-7 unused, bit 5 disables the cursor, bits 0-4 control the cursor shape
	out dx, al

	pop edx
	pop eax
	popf
	ret
   

; padding and magic number for bootable sector
times 510 - ($-$$) db 0 ; pad remainder of 510 bytes with 0
dw 0xAA55               ; boot signature (little-endian)
