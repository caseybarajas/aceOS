; boot.asm
; a very simple 16-bit bootloader for an ms-dos style os.
; this bootloader prints "bootloader loaded. welcome to caseyos!" to the screen.
; it then loads and calls a c kernel.

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

    ; set cursor position to top-left (row 0, col 0)
    mov ah, 0x02    ; bios video service: set cursor position
    mov bh, 0       ; page number 0
    mov dh, 0       ; row 0
    mov dl, 0       ; column 0
    int 0x10

    ; print the welcome message
    mov si, message ; point si to our message
print_char:
    lodsb           ; load byte from [si] into al, and increment si
    or al, al       ; check if al is null (end of string)
    jz load_kernel  ; if zero, jump to kernel loading

    mov ah, 0x0E    ; bios teletype output function
    mov bh, 0       ; page number 0
    mov bl, 0x07    ; white text on black background
    int 0x10        ; call bios video interrupt

    jmp print_char  ; loop to print next character

load_kernel:
    ; print kernel loading message
    mov si, kernel_loading_msg
    call print_string
    
    ; reset disk system
    xor ah, ah      ; function 0 - reset disk
    xor dl, dl      ; drive 0 (floppy)
    int 0x13
    jc disk_error   ; if carry flag is set, error occurred
    
    ; load the kernel (from sector 1, right after bootloader)
    mov ah, 0x02    ; BIOS read sector function
    mov al, 10      ; number of sectors to read (adjust as needed)
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
    
    ; Set up segment registers for kernel
    ; The kernel code is loaded at physical address 0x10000 (segment 0x1000:0)
    mov ax, 0x1000  ; Set all segments to where the kernel is loaded
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    ; Don't change SS:SP since we're still using the bootloader's stack
    
    ; jump to kernel
    jmp 0x1000:0x0000  ; far jump to kernel entry point

disk_error:
    mov si, disk_error_msg
    call print_string
    jmp halt_system

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

halt_system:
    cli             ; disable interrupts
    hlt             ; halt the processor
    jmp halt_system ; in case of interrupt, halt again

message:
    db "Bootloader loaded. Welcome to CaseyOS!", 13, 10, 0 ; with CR+LF

kernel_loading_msg:
    db "Loading kernel...", 13, 10, 0

kernel_loaded_msg:
    db "Kernel loaded successfully!", 13, 10, 0

disk_error_msg:
    db "Disk read error!", 13, 10, 0

; padding and magic number for bootable sector
times 510 - ($-$$) db 0 ; pad remainder of 510 bytes with 0
dw 0xAA55               ; boot signature (little-endian)
