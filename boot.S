.section .text
.code16
.intel_syntax noprefix

.global _start
_start:
    # BIOS normally enters at 0000:7c00. Normalize CS so all labels are
    # physical addresses and save the BIOS drive number before using DL.
    cli
    jmp 0x0000:start16

start16:
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7c00
    sti

    mov [boot_drive], dl
    mov si, msg_loading
    call bios_print

    # INT 13h extensions load the kernel at physical address 0x1000.
    # The image reserves 127 sectors after this boot sector, so the kernel
    # has room to grow without changing this first-stage loader.
    mov si, disk_address_packet
    mov dl, [boot_drive]
    mov ah, 0x42
    int 0x13
    jc disk_error

    call enable_a20

    cli
    lgdt [gdt_descriptor]
    mov eax, cr0
    or eax, 0x00000001
    mov cr0, eax
    jmp 0x08:protected_mode

bios_print:
    lodsb
    test al, al
    jz bios_print_done
    mov ah, 0x0e
    mov bh, 0x00
    int 0x10
    jmp bios_print
bios_print_done:
    ret

disk_error:
    mov si, msg_error
    call bios_print
    cli
.disk_halt:
    hlt
    jmp .disk_halt

enable_a20:
    # The fast A20 gate is supported by PC BIOSes and by UTM's emulated PC.
    in al, 0x92
    or al, 0x02
    and al, 0xfe
    out 0x92, al
    ret

.code32
protected_mode:
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov esp, 0x00090000
    xor ebp, ebp

    # kernel.bin is linked to and loaded at physical address 0x1000.
    call 0x00001000

.kernel_halt:
    cli
    hlt
    jmp .kernel_halt

.code16
.align 4
boot_drive:
    .byte 0

msg_loading:
    .asciz "xOS v1 Lite - loading..."
msg_error:
    .asciz "\r\nDisk read error. Halted."

# Disk Address Packet for INT 13h AH=42. Load 127 sectors from LBA 1 into
# 0100:0000 (physical 0x1000). The image builder pads the remaining sectors.
.align 4
disk_address_packet:
    .byte 0x10, 0x00
    .word 127
    .word 0x0000, 0x0100
    .long 1
    .long 0

# Flat 32-bit code/data segments, 4 GiB limit, page granularity.
.align 8
gdt_start:
    .quad 0x0000000000000000
    .quad 0x00cf9a000000ffff
    .quad 0x00cf92000000ffff
gdt_end:

gdt_descriptor:
    .word gdt_end - gdt_start - 1
    .long gdt_start
