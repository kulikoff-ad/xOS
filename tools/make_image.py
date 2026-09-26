#!/usr/bin/env python3
"""Create the fixed-size raw disk image used by xOS.

Sector zero is the BIOS boot sector. The kernel begins at LBA 1 and the
first-stage loader reads the following 127 sectors into physical 0x1000.
Padding the image makes it look like a normal small hard disk to UTM instead
of a strangely-sized concatenation of two files.
"""

from pathlib import Path
import sys

SECTOR = 512
KERNEL_SECTORS = 127
DISK_MIB = 16


def main() -> int:
    if len(sys.argv) != 4:
        print(f"usage: {sys.argv[0]} BOOT.BIN KERNEL.BIN OUTPUT.IMG", file=sys.stderr)
        return 2

    boot_path, kernel_path, output_path = map(Path, sys.argv[1:])
    boot = boot_path.read_bytes()
    kernel = kernel_path.read_bytes()

    if len(boot) != SECTOR:
        raise SystemExit(f"boot sector must be 512 bytes, got {len(boot)}")
    if boot[510:512] != b"\x55\xaa":
        raise SystemExit("boot sector has no 0xaa55 BIOS signature")
    if len(kernel) > KERNEL_SECTORS * SECTOR:
        raise SystemExit("kernel is larger than the 127 sectors loaded by boot.S")

    disk_size = DISK_MIB * 1024 * 1024
    image = bytearray(disk_size)
    image[:SECTOR] = boot
    image[SECTOR : SECTOR + len(kernel)] = kernel
    output_path.parent.mkdir(parents=True, exist_ok=True)
    output_path.write_bytes(image)
    print(
        f"created {output_path} ({DISK_MIB} MiB), "
        f"kernel {len(kernel)} bytes at LBA 1"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
