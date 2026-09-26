OUTPUT_FORMAT("binary")
OUTPUT_ARCH(i386)
ENTRY(_start)

SECTIONS
{
    . = 0x7c00;
    .text : ALIGN(1)
    {
        *(.text*)
        *(.rodata*)
        *(.data*)
    }

    /* A BIOS boot sector is exactly one 512-byte sector. */
    . = 0x7dfe;
    .signature :
    {
        SHORT(0xaa55)
    }
}
