#!/bin/sh
set -eu
cd "$(dirname "$0")/.."
command -v gcc >/dev/null || { echo "gcc is required" >&2; exit 1; }
command -v grub-mkrescue >/dev/null || { echo "grub-mkrescue is required (install grub-pc-bin grub-common xorriso)" >&2; exit 1; }
make clean >/dev/null 2>&1 || true
make
printf '\nBuilt: build/xos-v1-lite.iso\n'
