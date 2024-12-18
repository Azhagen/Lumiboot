.code16
.intel_syntax noprefix

.section .legacy.int05h, "axR", @progbits
.global int05h
.type   int05h, @function
int05h:
    sti
    push ds
    push di

    mov  di, 0xF000
    mov  ds, di

    call print

    pop  di
    pop  ds
    iret