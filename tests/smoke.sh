#!/bin/sh
set -eu
[ -f iso/boot/grub/grub.cfg ]
grep -q 'multiboot /boot/xos.bin' iso/boot/grub/grub.cfg
grep -q 'MAGIC' src/boot/boot.S
if [ -f build/xos.bin ]; then
  python3 - <<'PY'
p=open('build/xos.bin','rb').read(8192)
assert bytes.fromhex('02 b0 ad 1b') in p
PY
fi
echo 'xOS source smoke test: OK'
