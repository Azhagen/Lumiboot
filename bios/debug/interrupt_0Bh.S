.code16
.intel_syntax noprefix

.section .text.interrupt_0Bh, "ax", @progbits
.global interrupt_0Bh
.type   interrupt_0Bh, @function
interrupt_0Bh:
    push es
    push ds
    push ss
    pusha

    mov  ax, 0xF000
    mov  ds, ax

    mov  bp, sp
    push bp
    call uart_interrupt
    pop  bp

    popa
    pop  ss
    pop  es
    pop  ds
    iret