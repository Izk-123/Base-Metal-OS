; switch.asm – Context switching routine
; void do_switch(uint32_t *old_esp, uint32_t new_esp)
;
; Saves the current task's registers and stack pointer,
; then loads the new task's stack pointer and registers.
; Returns to the new task's instruction pointer.

[BITS 32]
[GLOBAL do_switch]

do_switch:
    ; Arguments:
    ;   [esp+4] = pointer to old_esp (where to save current ESP)
    ;   [esp+8] = new_esp (the target task's stack pointer)

    mov eax, [esp+4]      ; eax = &old_esp
    mov ecx, [esp+8]      ; ecx = new_esp

    ; Save callee‑saved registers (required by x86 calling convention)
    push ebp
    push ebx
    push esi
    push edi

    ; Save current stack pointer into the location pointed by eax
    mov [eax], esp

    ; Switch to the new stack
    mov esp, ecx

    ; Restore callee‑saved registers from the new stack
    pop edi
    pop esi
    pop ebx
    pop ebp

    ; Return – this will jump to the instruction pointer that was saved
    ; on the new stack (the return address from when the task was created
    ; or preempted).
    ret

; Mark the stack as non‑executable (avoids linker warnings)
section .note.GNU-stack noalloc noexec nowrite progbits