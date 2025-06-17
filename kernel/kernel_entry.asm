; kernel_entry.asm - Kernel entry point
; This file provides the entry point that the bootloader jumps to

[bits 32]                   ; We're in 32-bit protected mode

section .text
global _start               ; Make _start globally visible
extern kernel_main          ; Declare external C function

_start:
    ; Set up segment registers with proper data segment
    ; The bootloader GDT has: null(0x00), code(0x08), data(0x10)
    mov ax, 0x10            ; Data segment selector (0x10 = 16 decimal)
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    
    ; Set up stack pointer - use a safe area of memory
    mov esp, 0x90000        ; Stack at 576KB, growing downward
    mov ebp, esp            ; Set up frame pointer
    
    ; Clear direction flag for string operations
    cld
    
    ; Write a visible pattern to video memory to show we got here
    mov eax, 0xB8000        ; VGA text mode address
    mov word [eax], 0x0F41  ; White 'A' on black background
    mov word [eax+2], 0x0F42 ; White 'B' on black background
    mov word [eax+4], 0x0F43 ; White 'C' on black background
    mov word [eax+6], 0x0F44 ; White 'D' on black background
    
    ; Call the kernel main function
    call kernel_main
    
    ; If kernel_main ever returns, halt the system
halt_loop:
    cli                     ; Disable interrupts
    hlt                     ; Halt the processor
    jmp halt_loop           ; Loop in case of interrupt 