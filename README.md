# xOS v1 Lite

xOS Lite is a small, **freestanding i386 operating system**, not a Linux application. BIOS/GRUB loads `xos.bin`, which enters a 32-bit kernel and draws its desktop directly into the GRUB-provided 32-bit framebuffer.

## Build and boot

On Debian/Ubuntu install the toolchain once:

```sh
sudo apt install build-essential gcc-multilib grub-pc-bin grub-common xorriso qemu-system-x86
./scripts/build.sh
./scripts/run.sh
# or: qemu-system-i386 -cdrom build/xos-v1-lite.iso -m 64M
```

The result is `build/xos-v1-lite.iso`. A physical machine can boot the ISO from USB/DVD (use a tested image writer; do not copy it as a regular file). Legacy BIOS is the primary target. GRUB's `gfxpayload` requests 1024x768x32.

## What works in v1 Lite

* Multiboot 1 GRUB boot path and freestanding 32-bit kernel
* Direct linear framebuffer desktop: xOS top bar, Files card, Terminal window
* PS/2 keyboard polling, command line editing, and PS/2 mouse controller initialization/polling
* PIT timer initialization, tiny bump allocator, and a read-only virtual file layer
* Terminal commands: `help`, `clear`, `about`, `version`, `sysinfo`, `files`, `reboot`, `shutdown`
* QEMU/Bochs reset and power-off request paths

The Lite build deliberately has no libc, scheduler, disk driver, or background services. It is designed to remain usable on machines with 64 MB RAM (QEMU uses 64 MB in `run.sh`).

## Layout

`src/boot` contains the Multiboot entry and linker script; `src/kernel` is the kernel; `src/drivers`, `src/memory`, `src/filesystem`, `src/gui`, `src/terminal`, and `src/system` contain the low-level subsystems. `iso/boot/grub` is the ISO boot menu. `docs/` and `tests/` hold project notes and smoke checks.

## Roadmap: v1.1

Hardware IRQ/IDT and a real cursor, complete mouse movement/buttons, ATA/ISO read-only storage, a real heap and memory map reporting, CPU/RAM detection, window dragging, settings/about windows, and a safer ACPI shutdown implementation.
