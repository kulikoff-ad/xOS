#!/bin/sh
set -eu
cd "$(dirname "$0")/.."
[ -f build/xos-v1-lite.iso ] || ./scripts/build.sh
exec qemu-system-i386 -cdrom build/xos-v1-lite.iso -m 64M -serial stdio
