.code16
.intel_syntax noprefix

.section .legacy.int13h, "ax", @progbits
.global interrupt_13h
.type   interrupt_13h, @function
interrupt_13h:
    sti
    pusha
    push es
    push ds
    pushf

    mov  ax, 0xF000
    mov  ds, ax

    mov  bp, sp
    push bp
    call block_handler
    pop  bp

    popf
    pop  ds
    pop  es
    popa

    retf 2