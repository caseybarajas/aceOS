; boot.asm
; a very simple 16-bit bootloader for an ms-dos style os.
; this bootloader prints "bootloader loaded. welcome to caseyos!" to the screen.
; it then attempts to call a c kernel.

bits 16             ; we are in 16-bit real mode

ORG 0x7C00          ; bios loads our bootloader at this address

start:
    cli             ; disable interrupts initially
    mov ax, 0x07C0  ; set up data segment
    mov ds, ax
    mov es, ax
    mov ss, ax      ; set up stack segment
    mov sp, 0x7C00  ; stack pointer grows downwards from 0x7c00

    ; clear the screen (optional, but good practice)
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
    jz halt_bootloader ; if zero, jump to halt (or kernel load)

    mov ah, 0x0E    ; bios teletype output function
    mov bh, 0       ; page number 0
    mov bl, 0x07    ; white text on black background
    int 0x10        ; call bios video interrupt

    jmp print_char  ; loop to print next character

halt_bootloader:
    ; in a real scenario, you would load your kernel here.
    ; for now, we just halt.
    ; we'll add a call to a c kernel later.
    ; for now, let's assume kernel_main is at a known location if we were to load it.
    ; call 0x1000 ; example: if kernel was loaded at 0x1000:0000 (segment 0x1000)
    ; for this basic example, we'll just hang.

    cli             ; disable interrupts
    hlt             ; halt the processor

message:
    db "Bootloader loaded. Welcome to CaseyOS!", 0 ; null-terminated string

; padding and magic number for bootable sector
times 510 - ($-$$) db 0 ; pad remainder of 510 bytes with 0
dw 0xAA55               ; boot signature (little-endian)
