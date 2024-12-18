.code16
.intel_syntax noprefix

#define BDA_SEG  0x40
#define PIC_CMD  0x20
#define PIC_EOI  0x20

.section .legacy.int0Eh, "ax", @progbits
.global interrupt_0Eh
.type   interrupt_0Eh, @function
interrupt_0Eh:
    push ds
    push ax
    xor  ax, ax
    mov  ds, ax
    orb  [0x43E], 0x80
    mov  al, 0x20
    out  0x20, al
    pop  ax
    pop  ds
    iret