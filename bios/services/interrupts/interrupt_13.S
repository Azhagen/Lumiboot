.code16
.intel_syntax noprefix

.section .legacy.int13h, "ax", @progbits

.global interrupt_13h
.type   interrupt_13h, @function
interrupt_13h:
    pusha
    push es
    push ds
    pushf

    sti
    mov  ax, 0xF000
    mov  ds, ax

    mov  ax, sp
    push ax
    call block_handler
    pop  ax

    popf
    pop  ds
    pop  es
    popa

    retf 2