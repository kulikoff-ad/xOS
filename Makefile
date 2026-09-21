TARGET=xos-v1-lite
CC?=gcc
CFLAGS=-m32 -ffreestanding -fno-stack-protector -fno-pie -O2 -Wall -Wextra -Isrc -Isrc/drivers
LDFLAGS=-m32 -nostdlib -no-pie -T src/boot/linker.ld
SOURCES=src/kernel/kernel.c src/gui/gui.c src/terminal/terminal.c src/drivers/keyboard.c src/drivers/mouse.c src/memory/memory.c src/filesystem/fs.c src/system/system.c
OBJECTS=$(SOURCES:.c=.o) src/boot/boot.o
all: build/$(TARGET).iso
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@
src/boot/boot.o: src/boot/boot.S
	$(CC) -m32 -ffreestanding -c $< -o $@
build/xos.bin: $(OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $(OBJECTS)
build/$(TARGET).iso: build/xos.bin iso/boot/grub/grub.cfg
	mkdir -p build/isowork/boot/grub && cp build/xos.bin build/isowork/boot/xos.bin && cp iso/boot/grub/grub.cfg build/isowork/boot/grub/
	grub-mkrescue -o $@ build/isowork >/dev/null
clean:
	rm -f $(OBJECTS) build/xos.bin build/$(TARGET).iso
.PHONY: all clean
