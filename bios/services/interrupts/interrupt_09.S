.code16
.arch i8086
.intel_syntax noprefix

#define BDA_SEG  0x40
#define PIC_CMD  0x20
#define PIC_EOI  0x20

.section .legacy.int09h, "ax", @progbits
.global interrupt_09h
.type   interrupt_09h, @function
interrupt_09h:
    push ds
    push di

    mov  di, 0xF000
    mov  ds, di

    push ax
    push bx
    push cx
    push dx

    call keyboard_push_key

    pop  dx
    pop  cx
    pop  bx
    pop  ax

    pop  di
    pop  ds
    iret