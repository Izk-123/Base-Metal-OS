[BITS 32]
[GLOBAL do_switch]

do_switch:
    mov eax, [esp+4]
    mov ecx, [esp+8]
    push ebp
    push ebx
    push esi
    push edi
    mov [eax], esp
    mov esp, ecx
    pop edi
    pop esi
    pop ebx
    pop ebp
    ret

section .note.GNU-stack noalloc noexec nowrite progbits