[BITS 32]

global _start
extern kernel_main

section .text

_start:
    mov esp, stack_top

    mov al, 'Y'
    mov dx, 0x3F8
    out dx, al

    call kernel_main
    jmp $

section .bss
align 16
stack_bottom:
    resb 4096
stack_top: