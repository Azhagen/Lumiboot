.code16
.arch i8086
.intel_syntax noprefix

.section .legacy.int11h, "ax", @progbits
.global interrupt_11h
.type   interrupt_11h, @function
interrupt_11h:
    push ds
    xor  ax, ax
    mov  ds, ax
    mov  ax, [0x410]
    pop  ds
    iret