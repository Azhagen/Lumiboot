.code16
.intel_syntax noprefix

.section .legacy.int17h, "ax", @progbits
.global interrupt_17h
.type   interrupt_17h, @function
interrupt_17h:
    pusha
    push es
    push ds
    pushf

    sti
    mov  si, 0xF000
    mov  ds, si

    mov  si, sp
    push si
    call printer_handler
    pop  si

    popf
    pop  ds
    pop  es
    popa
    iret