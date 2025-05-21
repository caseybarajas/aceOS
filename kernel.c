// kernel.c
// a very minimal c kernel stub for caseyos.

// function to clear the screen using bios video interrupt
// this is a c-callable wrapper for the assembly instruction.
// note: for a real kernel, direct hardware access or more complex video drivers would be used.
// this is a simplification for early stages.
void clear_screen() {
    // this is a placeholder. in a real c kernel linked with the bootloader,
    // you'd call an assembly function or use inline assembly to trigger int 10h.
    // example (conceptual, won't compile standalone without proper setup):
    // __asm__ __volatile__ (
    //     "mov ah, 0x00;"
    //     "mov al, 0x03;" // text mode 80x25
    //     "int 0x10;"
    // );
}

// function to print a character to the screen
// this is also a placeholder for calling bios tty output.
void k_print_char(char c, unsigned char attribute, int row, int col) {
    // placeholder for int 10h, ah=0x0e (teletype output)
    // or ah=0x09 (write character and attribute at cursor)
    // this requires setting up segments and calling interrupts from c,
    // which is complex without a proper c runtime/environment.
}

// function to print a string to the screen
void k_print_string(const char *str, unsigned char attribute, int row, int start_col) {
    int col = start_col;
    while (*str) {
        // k_print_char(*str, attribute, row, col); // conceptual
        // for now, we can't directly call bios from this c stub easily
        // without more infrastructure.
        str++;
        col++;
        if (col >= 80) { // basic line wrapping
            row++;
            col = 0;
        }
        if (row >= 25) { // basic screen scroll (not implemented here)
            row = 24; // stay on last line
        }
    }
}


// main function for the kernel
void kernel_main() {
    // clear_screen(); // conceptual call

    const char *message = "C Kernel Loaded!";
    
    // at this stage, printing directly from c to screen is complex
    // without setting up a lot more infrastructure (like a vga driver or
    // proper bios interrupt wrappers accessible from c).
    // the bootloader has already printed its message.
    // this function being called signifies the kernel has "loaded".

    // for now, the kernel doesn't do much more.
    // future steps:
    // 1. initialize global descriptor table (gdt)
    // 2. initialize interrupt descriptor table (idt)
    // 3. setup programmable interrupt controller (pic)
    // 4. enable interrupts
    // 5. initialize basic drivers (keyboard, timer)
    // 6. implement a simple shell

    while (1) {
        // infinite loop to keep the kernel running
        // (or rather, to prevent it from returning and crashing)
    }
}
