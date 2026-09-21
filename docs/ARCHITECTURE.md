# Architecture

GRUB Multiboot 1 supplies the framebuffer address, pitch, dimensions, and memory information to `_start`. The assembly stub installs a 16 KiB stack and calls `kmain`. The kernel initializes the PIT and PS/2 controller, then runs a deliberately simple polling loop. The GUI writes 32-bit pixels directly; no host OS, SDL, libc, or userspace process is involved.
