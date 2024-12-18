.code16
.intel_syntax noprefix

#define BDA_SEGM    0x40   // BDA segment

.section .legacy.int10h, "ax", @progbits

.global interrupt_10h
.type   interrupt_10h, @function
interrupt_10h:
    push ds
    push es
    push bp
    push di

    cmp  ah, 0x10
    jae  interrupt_10h_end

    mov  di, 0xF000
    mov  ds, di

    push dx
    push cx
    push bx
    push ax

    mov  bp, sp
    push bp
    call video_handler
    pop  bp

    pop  ax
    pop  bx
    pop  cx
    pop  dx

interrupt_10h_end:
    pop  di
    pop  bp
    pop  es
    pop  ds
    iret