# xOS - a tiny BIOS-bootable 32-bit operating system
# The build intentionally uses only the host GCC/binutils and Python 3.

CC      ?= gcc
LD      ?= ld
OBJCOPY ?= objcopy
PYTHON  ?= python3

BUILD := build

CFLAGS := -m32 -ffreestanding -fno-pie -fno-stack-protector \
          -fno-asynchronous-unwind-tables -fno-unwind-tables \
          -fno-builtin -fno-common -mno-mmx -mno-sse -O2 \
          -Wall -Wextra -Werror
ASFLAGS := -m32 -ffreestanding -fno-pie -fno-asynchronous-unwind-tables
LDFLAGS := -m elf_i386

.PHONY: all image clean run check

all: image

image: $(BUILD)/xos.img

$(BUILD):
	mkdir -p $(BUILD)

$(BUILD)/boot.o: boot.S | $(BUILD)
	$(CC) $(ASFLAGS) -c $< -o $@

$(BUILD)/boot.bin: $(BUILD)/boot.o boot.ld
	$(LD) $(LDFLAGS) -T boot.ld -o $@ $(BUILD)/boot.o
	@test "$$(wc -c < $@ | tr -d ' ')" -eq 512 || (echo "boot sector is not 512 bytes"; exit 1)

$(BUILD)/kernel_entry.o: kernel_entry.S | $(BUILD)
	$(CC) $(ASFLAGS) -c $< -o $@

$(BUILD)/kernel.o: kernel.c | $(BUILD)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD)/kernel.elf: $(BUILD)/kernel_entry.o $(BUILD)/kernel.o kernel.ld
	$(LD) $(LDFLAGS) -T kernel.ld -o $@ $(BUILD)/kernel_entry.o $(BUILD)/kernel.o

$(BUILD)/kernel.bin: $(BUILD)/kernel.elf
	$(OBJCOPY) -O binary $< $@
	@test "$$(wc -c < $@ | tr -d ' ')" -le 65024 || (echo "kernel is larger than the boot loader limit"; exit 1)

$(BUILD)/xos.img: $(BUILD)/boot.bin $(BUILD)/kernel.bin tools/make_image.py
	$(PYTHON) tools/make_image.py $(BUILD)/boot.bin $(BUILD)/kernel.bin $@

# Optional convenience target. UTM is the primary target documented in README.md.
run: image
	@if command -v qemu-system-i386 >/dev/null 2>&1; then \
		exec qemu-system-i386 -drive format=raw,file=$(BUILD)/xos.img; \
	else \
		echo "qemu-system-i386 is not installed; open build/xos.img in UTM."; \
		exit 1; \
	fi

check: image
	@$(PYTHON) tools/check_image.py

clean:
	rm -rf $(BUILD)
