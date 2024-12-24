.code16
.intel_syntax noprefix

#define BDA_SEG  0x40
#define PIC_CMD  0x20
#define PIC_EOI  0x20

.section .legacy.int09h, "ax", @progbits
.global interrupt_09h
.type   interrupt_09h, @function
interrupt_09h:
    pusha
    push es
    push ds
    pushf

    mov  di, 0xF000
    mov  ds, di
    call keyboard_push_key

    popf
    pop  ds
    pop  es
    popa

    iret