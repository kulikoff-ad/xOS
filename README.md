# xOS

xOS v1 Lite is now a **real bootable 32-bit x86 operating system**, not only a
project description. It contains a BIOS boot sector, a protected-mode kernel,
VGA text output, a PS/2 keyboard driver, an RTC clock reader, and an interactive
shell. The kernel is freestanding: after the BIOS loads it, xOS runs directly
on the emulated machine without Linux, macOS, DOS, GRUB, or a C runtime below
it.

The current v1 interface is a fast text desktop. It is intentionally small and
works well on old or low-end hardware. It is not yet a multitasking POSIX OS or
a pixel GUI.

## Build

On Linux, WSL, or another Unix-like host install:

- GCC with 32-bit assembler support
- GNU `ld` and `objcopy`
- Python 3
- `make`

Then run from the repository root:

```sh
make
make check
```

The result is `build/xos.img`, a 16 MiB raw BIOS disk image. The image has a
512-byte boot sector followed by the kernel at LBA 1. The first-stage loader
uses BIOS INT 13h extensions, enables A20, switches to 32-bit protected mode,
and jumps to the kernel at physical address `0x1000`.

If your host compiler does not support `-m32`, install its multilib/32-bit
compiler package. No 32-bit system libraries are required; all code is linked
freestanding. macOS's Apple `ld` is not the GNU ELF linker used by this
Makefile; use a cross toolchain such as `i686-elf-gcc`, `i686-elf-ld`, and
`i686-elf-objcopy` (or build in Linux/WSL), for example:

```sh
make clean
make CC=i686-elf-gcc LD=i686-elf-ld OBJCOPY=i686-elf-objcopy check
```

A QEMU installation can run it with:

```sh
make run
```

`make check` verifies the boot signature and image layout. Generated files stay
in `build/` and are ignored by Git.

## Use it in UTM

1. Run `make` on the host and keep `build/xos.img` available.
2. Open UTM and choose **Create a New Virtual Machine**.
3. Choose **Emulate** / **System** (not ARM virtualization).
4. Set **Architecture** to **x86_64**. On Apple Silicon this is required: xOS
   is an x86 BIOS image and must be emulated, not run as an ARM64 guest.
5. Use these stable settings:

   | UTM option | Value |
   |---|---|
   | System / architecture | x86_64 PC |
   | Firmware | Legacy BIOS / SeaBIOS; EFI/UEFI off |
   | Memory | 64 MiB (128 MiB is also fine) |
   | CPU cores | 1 |
   | Storage | Existing disk image: `build/xos.img` |
   | Disk interface | IDE (or ATA), not VirtIO/NVMe |
   | Boot order | The xOS disk first |
   | Display | VGA-compatible default display |
   | Network / sound / USB | Optional; xOS v1 does not use them |

6. Start the VM. You should see `xOS v1 Lite - loading...`, followed by the
   xOS shell. Click the VM display and type commands.

The most reliable UTM setup is a new **x86_64 System Emulation** VM with the
raw image attached as an IDE hard disk. On an Intel Mac, x86_64 virtualization
is also possible if UTM exposes a legacy BIOS, but emulation is the portable
choice. Do not create an ARM64 VM for this image.

### If UTM says “no bootable device”

- Confirm that `make check` passes and that the selected file is exactly
  `build/xos.img`, not `kernel.bin`.
- Change the disk bus to IDE/ATA.
- Turn off EFI/UEFI and select the disk as the first boot device.
- On Apple Silicon, recreate the VM as **x86_64 emulation**, not ARM64.

## Shell commands

After boot, press Enter after each command:

```text
help                  list commands
about                 show the kernel description
info                  show architecture and memory information
files                 list built-in read-only files
cat readme.txt        read a file
settings              show the machine settings
time                  read the emulated RTC
echo hello            print text
desktop               redraw the text desktop
clear                 clear the screen
reboot                reboot the VM
halt                  halt the CPU
```

The keyboard driver uses the standard US PC layout. `Ctrl+L` clears the
screen. The built-in files are compiled into the kernel; a writable filesystem,
mouse driver, and pixel GUI are deliberately left for later xOS versions.

## Repository layout

- `boot.S` / `boot.ld` — 512-byte BIOS loader and GDT setup
- `kernel_entry.S` — protected-mode kernel entry
- `kernel.c` — VGA terminal, keyboard input, RTC, and shell
- `kernel.ld` — freestanding kernel linker layout at `0x1000`
- `tools/make_image.py` — raw disk image builder
- `Makefile` — reproducible build and image checks
