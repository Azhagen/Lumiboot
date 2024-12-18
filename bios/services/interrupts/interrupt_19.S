.code16
.arch i8086
.intel_syntax noprefix

.section .legacy.int19h, "ax", @progbits
.global interrupt_19h
.type   interrupt_19h, @function
interrupt_19h:
    jmp  do_interrupt_19h

.section .text.do_interrupt_19h, "ax", @progbits
.global do_interrupt_19h
.type   do_interrupt_19h, @function
do_interrupt_19h:
    push ds
    push es
    push bp
    push di

    mov  di, 0xF000
    mov  ds, di

    call boot_handler

    pop  di
    pop  bp
    pop  es
    pop  ds
    iret