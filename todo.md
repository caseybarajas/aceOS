User/kernel separation (GDT/TSS, ring3, copy-on-write basics).
On-disk filesystem (simple FAT-like or custom) atop ATA driver.
ELF loader + userspace “hello” using current syscall layer.
Scheduler polish (round‑robin quantum tuning, sleep/wake, priorities).
VFS layer to unify RAM FS now and disk FS later.