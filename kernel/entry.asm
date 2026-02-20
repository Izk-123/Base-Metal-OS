section .multiboot
align 8
    dd 0xe85250d6          ; magic
    dd 0                   ; architecture (x86)
    dd header_end - header_start
    dd -(0xe85250d6 + 0 + (header_end - header_start))

header_start:
    dw 0
    dw 0
    dd 8
header_end:

section .text
global _start
extern kernel_main

_start:
    call kernel_main
    hlt