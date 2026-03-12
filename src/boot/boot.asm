; boot.asm - Multiboot entry point
; GRUB needs a special "magic" header to recognize this as a bootable kernel

MBALIGN  equ 1 << 0          ; align loaded modules on page boundaries
MEMINFO  equ 1 << 1          ; provide memory map
FLAGS    equ MBALIGN | MEMINFO
MAGIC    equ 0x1BADB002      ; GRUB's magic number - don't change this!
CHECKSUM equ -(MAGIC + FLAGS) ; checksum so GRUB knows header is valid

section .multiboot           ; special section GRUB looks for
align 4
    dd MAGIC
    dd FLAGS
    dd CHECKSUM

section .bss                 ; uninitialised data (zeroed at boot)
align 16
stack_bottom:
    resb 16384               ; reserve 16KB for the kernel stack
stack_top:

section .text
global _start                ; make _start visible to the linker

_start:
    ; GRUB left us in 32-bit protected mode
    ; First thing: set up our stack
    mov esp, stack_top       ; point stack pointer to top of our stack

    ; Call our C kernel main function
    extern kernel_main
    call kernel_main

    ; If kernel_main ever returns, halt the CPU forever
    cli                      ; disable interrupts
.hang:
    hlt                      ; halt
    jmp .hang                ; loop in case of NMI

; Load GDT and reload segment registers
[GLOBAL gdt_flush]
gdt_flush:
    mov eax, [esp+4]   ; get pointer to GDT
    lgdt [eax]         ; load it
    mov ax, 0x10       ; 0x10 = kernel data segment offset in GDT
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    jmp 0x08:.flush    ; 0x08 = kernel code segment, far jump reloads CS
.flush:
    ret

; Load IDT
[GLOBAL idt_flush]
idt_flush:
    mov eax, [esp+4]
    lidt [eax]
    ret