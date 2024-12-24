.code16
.intel_syntax noprefix

.section .legacy.int15h, "ax", @progbits
.global interrupt_15h
.type   interrupt_15h, @function
interrupt_15h:
    sti

    pusha
    push es
    push ds
    pushf

    mov  bp, 0xF000
    mov  ds, bp

    mov  bp, sp
    push bp
    call system_handler
    pop  bp

    popf
    pop  ds
    pop  es
    popa

    retf 2