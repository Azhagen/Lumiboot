.code16
.intel_syntax noprefix

.section .legacy.int15h, "ax", @progbits
.global interrupt_15h
.type   interrupt_15h, @function
interrupt_15h:
    sti

    push ds
    push bp

    mov  bp, 0xF000
    mov  ds, bp

    pushf
    push es
    push di
    push si
    push dx
    push cx
    push bx
    push ax

    mov  bp, sp
    push bp

    call system_handler
    pop  bp

    pop  ax
    pop  bx
    pop  cx
    pop  dx
    pop  si
    pop  di
    pop  es
    popf

    pop  bp
    pop  ds

    retf 2