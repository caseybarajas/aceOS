CaseyOS: Design Notes & Rules
Project Goal

To create a very basic, MS-DOS style operating system from scratch for learning purposes.
The initial focus is on understanding boot procedures, low-level hardware interaction, and fundamental OS concepts.
Phase 1: The Bootloader

    Language: 16-bit x86 Assembly (NASM).

    Functionality:

        Initialize segments (DS, ES, SS, SP).

        Clear the screen (BIOS INT 10h).

        Set cursor position (BIOS INT 10h).

        Print a welcome message to the screen (BIOS INT 10h, teletype).

        [Future] Load the C kernel from the floppy disk image into memory.

        [Future] Jump to the C kernel's entry point.

    Constraints: Must fit within a single 512-byte boot sector.

    Testing: Use QEMU to emulate an i386 PC booting from a floppy image.

Phase 2: Minimal C Kernel

    Language: C (with potential inline assembly).

    Toolchain Goal: Find or set up a GCC cross-compiler for 16-bit real mode (e.g., i386-elf) or figure out how to compile C to be callable from the 16-bit bootloader. This is a known challenge.

    Initial Functionality (Stub):

        A kernel_main() function.

        Placeholder for printing a "Kernel Loaded" message (actual printing might be deferred until basic screen I/O is set up in C or via assembly helpers).

    Future Kernel Functionality (Long Term):

        GDT/IDT Setup: Transition to 32-bit protected mode (optional, but common for more advanced features). If staying 16-bit, manage interrupts carefully.

        Interrupt Handling: Basic keyboard and timer interrupts.

        Memory Management: Very simple (e.g., a basic heap, track free memory).

        Filesystem: Rudimentary FAT12-like reader for floppy disks.

        Shell/Command Interpreter: Basic commands like DIR, TYPE, CLS, RUN <program>.

        Simple Device Drivers: Keyboard, Screen (text mode).

Coding Style & Rules

    Comments: Comment extensively, especially for assembly and low-level C code. Explain why, not just what.

    Simplicity: Start with the simplest possible thing that works and build incrementally.

    Modularity (Future): Try to keep components (bootloader, kernel, drivers, shell) as separate as possible.

    Error Handling: For now, basic error handling might just be halting or printing an error message.

    Version Control: Use Git from the beginning!

    Resource Management: Be mindful of memory and CPU cycles, especially in the bootloader and early kernel.

In-OS Cursor Handling Rules (For CaseyOS Interface)

This section defines how the text cursor should behave within the CaseyOS environment (e.g., in the command shell or any text-based utilities).

    Visibility:

        The cursor should always be visible when input is expected.

        Consider a blinking block cursor (e.g., BIOS default or custom).

    Shape:

        Default: Underscore _ or full block █.

        Consistency: Use the same cursor shape across all parts of the OS shell/utilities.

    Movement:

        Arrow Keys: Left, Right, Up, Down should move the cursor logically within the current input field or viewable text area.

        Home/End: Move to the beginning/end of the current line of input.

        PageUp/PageDown: (If applicable, for text viewers) Scroll content and move the cursor.

        Backspace: Deletes the character to the left of the cursor and moves the cursor one position left.

        Delete: Deletes the character at the cursor position (cursor does not move, characters to the right shift left).

    Bounds:

        The cursor should not be allowed to move outside the defined text input area or screen boundaries in a way that causes errors.

        Wrap-around behavior at screen edges for command input should be defined (e.g., does it go to the next line?).

    Insertion vs. Overtype Mode:

        Default to Insertion Mode: New characters are inserted at the cursor position, shifting existing characters to the right.

        (Optional Future Feature) Consider an Overtype Mode, possibly toggled by an Insert key.

    Control via BIOS/Hardware:

        Initial cursor control will be via BIOS interrupts (e.g., INT 10h, AH=02h to set position, INT 10h, AH=01h to set shape).

        As the OS develops, direct VGA manipulation might be used for more control if moving beyond BIOS.

    Cursor Position Update:

        The OS must keep track of the logical cursor position (row, column).

        After any character output or cursor movement, the physical cursor on screen must be updated.

Code Editor (IDE/Text Editor) Cursor & Editing Preferences

This section outlines preferred settings and behaviors for the code editor used to develop CaseyOS. These are guidelines for the development environment, not for CaseyOS itself.

    Cursor Style:

        Shape: Block ( █ ) or I-beam ( | ). (Specify your preference)

        Blinking: Enabled. Helps in locating the cursor quickly.

    Indentation:

        Tabs vs. Spaces: (Choose one and stick to it, e.g., "Spaces only")

        Tab Width/Indent Size: e.g., 4 spaces.

        Auto-Indent: Enabled. The editor should automatically indent new lines based on the previous line's indentation.

    Whitespace:

        Show Whitespace: Optionally enable visibility for spaces and tabs to help identify mixed indentation or trailing whitespace.

        Trim Trailing Whitespace on Save: Enabled. Automatically removes unnecessary whitespace at the end of lines.

    Line Endings:

        Default Line Ending: LF (Unix-style) or CRLF (Windows-style). Choose one for consistency across project files (LF is common for cross-platform projects).

    Code Folding:

        Enabled. Useful for collapsing blocks of code (functions, loops, comments) to get a better overview.

    Syntax Highlighting:

        Essential. Must be enabled and correctly configured for Assembly (NASM dialect) and C.

    Auto-Completion/IntelliSense:

        If available for the languages, enable basic auto-completion for keywords and known symbols.

    Matching Brackets/Parentheses:

        Highlight matching pairs ( (), {}, [] ).

        Auto-closing pairs: Optional (e.g., typing ( automatically inserts )).

    Line Numbers:

        Always visible.

    Word Wrap:

        Typically Off for code, to maintain explicit line breaks. Long lines can be managed with horizontal scrolling or by breaking them manually.

    Mouse Behavior:

        Cursor Placement: Standard click-to-place.

        Selection: Standard click-and-drag for text selection.

        Column (Block) Selection: If available (often Alt+Mouse Drag), can be useful.

    Find and Replace:

        Robust find/replace functionality with options for case sensitivity, whole word, and regular expressions.

    Font:

        Monospaced font (e.g., Consolas, Fira Code, DejaVu Sans Mono, Source Code Pro).

        Clear distinction between similar characters (e.g., 0 vs O, 1 vs l vs I).

    Color Scheme:

        A comfortable color scheme with good contrast for readability (e.g., a "Dark" theme or a high-contrast "Light" theme).

Development Environment

    Emulator: QEMU (qemu-system-i386)

    Assembler: NASM

    C Compiler: GCC (need to configure for 16-bit or cross-compile for i386-elf)

    Linker: LD (from binutils)

    Debugger: GDB (with QEMU's GDB stub) and Bochs' internal debugger.

    Hex Editor: For inspecting binary images.

    Build System: GNU Make

Learning Resources

    OSDev Wiki (wiki.osdev.org)

    Intel Manuals

    "Operating System Concepts" (Silberschatz, Galvin, Gagne) - for theory

    Various online tutorials on bootloader development and OS basics.

    MikeOS, James Molloy's Kernel Development Tutorials, BrokenThorn Entertainment OS Tutorials.