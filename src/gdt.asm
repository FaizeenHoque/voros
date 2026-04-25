global gdt_flush

gdt_flush:
    lgdt [rdi]        ; rdi holds the pointer to gdtr (System V calling convention)

    mov ax, 0x10      ; 0x10 = index 2 in GDT (kernel data), shifted left by 3
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    pop rdi           ; grab return address off the stack
    push 0x08         ; 0x08 = index 1 in GDT (kernel code), shifted left by 3
    push rdi          ; push return address back
    retfq             ; far return — reloads CS with 0x08
