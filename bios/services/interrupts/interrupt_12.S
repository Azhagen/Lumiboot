.code16
.arch i8086
.intel_syntax noprefix

.section .legacy.int12h, "ax", @progbits
.global interrupt_12h
.type   interrupt_12h, @function
interrupt_12h:
    push ds
    xor  ax, ax
    mov  ds, ax
    mov  ax, [0x413]
    pop  ds
    iret
