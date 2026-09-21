/* Multiboot 1 entry: GRUB loads the kernel and hands us the info table. */
.set ALIGN,    1<<0
.set MEMINFO,  1<<1
.set VIDEO,    1<<2
.set FLAGS,    ALIGN | MEMINFO | VIDEO
.set MAGIC,    0x1BADB002
.set CHECKSUM, -(MAGIC + FLAGS)
.section .multiboot
.align 4
.long MAGIC, FLAGS, CHECKSUM
.long 0, 0, 0, 0, 1024, 768, 32
.section .bss
.align 16
stack_bottom:
.skip 16384
stack_top:
.section .text
.global _start
.type _start,@function
_start:
  mov $stack_top, %esp
  push %ebx
  push %eax
  call kmain
halt:
  cli
  hlt
  jmp halt
.size _start, .-_start
