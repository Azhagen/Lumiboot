.code16
.intel_syntax noprefix

#define PIC_CMD1    0x20
#define PIC_CMD2    0xA0

#define PIC_EOI     0x20

.section .text.int70h, "ax", @progbits
.global interrupt_70h
.type   interrupt_70h, @function
interrupt_70h:
    mov  al, PIC_EOI
    out  PIC_CMD1, al
    out  PIC_CMD2, al
    iret