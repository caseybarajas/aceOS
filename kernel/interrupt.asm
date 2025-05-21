[bits 32]

; This macro creates a stub for an interrupt handler that doesn't pass an error code
%macro ISR_NO_ERROR_CODE 1
global isr%1
isr%1:
    cli                 ; Disable interrupts
    push 0              ; Push dummy error code (doesn't exist for these interrupts)
    push %1             ; Push the interrupt number
    jmp isr_common_stub ; Go to common handler
%endmacro

; This macro creates a stub for an interrupt handler that does pass an error code
%macro ISR_ERROR_CODE 1
global isr%1
isr%1:
    cli                 ; Disable interrupts
    push %1             ; Push the interrupt number
    jmp isr_common_stub ; Go to common handler
%endmacro

; Function to load the IDT
global idt_load
idt_load:
    push ebp
    mov ebp, esp
    mov eax, [ebp+8]    ; Get the argument (IDT descriptor pointer)
    lidt [eax]          ; Load the IDT
    pop ebp
    ret

; CPU-generated interrupts (0-31)
ISR_NO_ERROR_CODE 0    ; Division by zero
ISR_NO_ERROR_CODE 1    ; Debug
ISR_NO_ERROR_CODE 2    ; Non-maskable interrupt
ISR_NO_ERROR_CODE 3    ; Breakpoint
ISR_NO_ERROR_CODE 4    ; Overflow
ISR_NO_ERROR_CODE 5    ; Bound range exceeded
ISR_NO_ERROR_CODE 6    ; Invalid opcode
ISR_NO_ERROR_CODE 7    ; Device not available
ISR_ERROR_CODE    8    ; Double fault
ISR_NO_ERROR_CODE 9    ; Coprocessor segment overrun
ISR_ERROR_CODE    10   ; Invalid TSS
ISR_ERROR_CODE    11   ; Segment not present
ISR_ERROR_CODE    12   ; Stack-segment fault
ISR_ERROR_CODE    13   ; General protection fault
ISR_ERROR_CODE    14   ; Page fault
ISR_NO_ERROR_CODE 15   ; Reserved
ISR_NO_ERROR_CODE 16   ; x87 floating-point exception
ISR_ERROR_CODE    17   ; Alignment check
ISR_NO_ERROR_CODE 18   ; Machine check
ISR_NO_ERROR_CODE 19   ; SIMD floating-point exception
ISR_NO_ERROR_CODE 20   ; Virtualization exception
ISR_ERROR_CODE    21   ; Control protection exception
ISR_NO_ERROR_CODE 22   ; Reserved
ISR_NO_ERROR_CODE 23   ; Reserved
ISR_NO_ERROR_CODE 24   ; Reserved
ISR_NO_ERROR_CODE 25   ; Reserved
ISR_NO_ERROR_CODE 26   ; Reserved
ISR_NO_ERROR_CODE 27   ; Reserved
ISR_NO_ERROR_CODE 28   ; Reserved
ISR_NO_ERROR_CODE 29   ; Reserved
ISR_ERROR_CODE    30   ; Reserved
ISR_NO_ERROR_CODE 31   ; Reserved

; External interrupts (IRQs 0-15)
ISR_NO_ERROR_CODE 32   ; IRQ0 - Timer
ISR_NO_ERROR_CODE 33   ; IRQ1 - Keyboard
ISR_NO_ERROR_CODE 34   ; IRQ2 - Cascade for 8259A Slave controller
ISR_NO_ERROR_CODE 35   ; IRQ3 - COM2
ISR_NO_ERROR_CODE 36   ; IRQ4 - COM1
ISR_NO_ERROR_CODE 37   ; IRQ5 - LPT2
ISR_NO_ERROR_CODE 38   ; IRQ6 - Floppy disk
ISR_NO_ERROR_CODE 39   ; IRQ7 - LPT1 / Unreliable spurious interrupt
ISR_NO_ERROR_CODE 40   ; IRQ8 - CMOS real-time clock
ISR_NO_ERROR_CODE 41   ; IRQ9 - Free / Legacy SCSI / NIC
ISR_NO_ERROR_CODE 42   ; IRQ10 - Free / SCSI / NIC
ISR_NO_ERROR_CODE 43   ; IRQ11 - Free / SCSI / NIC
ISR_NO_ERROR_CODE 44   ; IRQ12 - PS/2 Mouse
ISR_NO_ERROR_CODE 45   ; IRQ13 - FPU / Coprocessor / Inter-processor
ISR_NO_ERROR_CODE 46   ; IRQ14 - Primary ATA hard disk
ISR_NO_ERROR_CODE 47   ; IRQ15 - Secondary ATA hard disk

; This is the common part for all interrupt handlers
extern isr_handler      ; The C function to handle interrupts
isr_common_stub:
    ; Save all registers
    pusha
    
    ; Save data segment selector
    mov ax, ds
    push eax
    
    ; Set kernel data segment selector
    mov ax, 0x10        ; Kernel data segment selector
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    
    ; Call C interrupt handler
    call isr_handler
    
    ; Restore data segment selector
    pop eax
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    
    ; Restore registers and clean up
    popa
    add esp, 8          ; Clean error code and interrupt number
    sti                 ; Re-enable interrupts
    iret                ; Return from interrupt 