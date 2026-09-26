.section .text
.code32
.intel_syntax noprefix

.global _start
.extern kmain
.extern __bss_start
.extern __bss_end

_start:
    cli
    cld
    mov esp, 0x00090000
    xor ebp, ebp

    # A raw binary has no bytes for .bss, so initialize it explicitly.
    mov edi, __bss_start
    mov ecx, __bss_end
    sub ecx, edi
    xor eax, eax
    rep stosb

    call kmain

	.halt:
    hlt
    jmp .halt

.section .note.GNU-stack,"",@progbits
