.code16
.arch i8086
.intel_syntax noprefix

.section .text, "ax", @progbits

.global interrupt_18h
.type   interrupt_18h, @function
interrupt_18h:
    push ds
    push es
    push bp
    push di

    mov  di, 0xF000
    mov  ds, di

    call fail_handler

    pop  di
    pop  bp
    pop  es
    pop  ds
    iret