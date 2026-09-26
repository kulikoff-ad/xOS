OUTPUT_FORMAT(elf32-i386)
OUTPUT_ARCH(i386)
ENTRY(_start)

PHDRS
{
    text PT_LOAD FLAGS(5);    /* read + execute */
    rodata PT_LOAD FLAGS(4);  /* read only */
    data PT_LOAD FLAGS(6);    /* read + write */
}

SECTIONS
{
    . = 0x1000;

    .text : ALIGN(16)
    {
        KEEP(*(.text._start))
        *(.text*)
    } :text

    .rodata : ALIGN(16)
    {
        *(.rodata*)
    } :rodata

    .data : ALIGN(16)
    {
        *(.data*)
    } :data

    .bss : ALIGN(16)
    {
        __bss_start = .;
        *(COMMON)
        *(.bss*)
        __bss_end = .;
    } :data

    /DISCARD/ :
    {
        *(.comment*)
        *(.eh_frame*)
        *(.note*)
        *(.rel*)
    }
}
