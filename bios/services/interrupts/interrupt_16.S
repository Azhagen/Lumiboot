.code16
.intel_syntax noprefix

.section .legacy.int16h, "ax", @progbits
.global interrupt_16h
.type   interrupt_16h, @function
interrupt_16h:
    sti

    pusha
    push es
    push ds
    pushf

    mov  di, 0xF000
    mov  ds, di

    mov  di, sp
    push di
    call keyboard_handler
    pop  di

    popf
    pop  ds
    pop  es
    popa

    retf 2