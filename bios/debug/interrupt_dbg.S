.code16
.intel_syntax noprefix

.section .text.interrupt_dbg, "ax", @progbits
.global interrupt_dbg
.type   interrupt_dbg, @function
interrupt_dbg:
    push es
    push ds
    push ss
    pusha

    mov  ax, 0xF000
    mov  ds, ax

    mov  bp, sp
    push bp
    call trap_interrupt
    pop  bp

    popa
    pop  ss
    pop  es
    pop  ds
    iret