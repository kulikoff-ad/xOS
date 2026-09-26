#!/usr/bin/env python3
from pathlib import Path

image = Path("build/xos.img").read_bytes()
assert image[510:512] == b"\x55\xaa", "missing BIOS boot signature"
assert image[512:516], "kernel is missing"
print(f"OK: {len(image)} byte xOS disk image, BIOS signature present")
