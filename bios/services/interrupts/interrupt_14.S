.code16
.arch i8086
.intel_syntax noprefix

#define BDA_SEG  0x40
#define BDA_COM  0x00

#define UART_LCR 3
#define UART_MCR 4
#define UART_LSR 5
#define UART_MSR 6

.section .legacy.int14h, "ax", @progbits
.global interrupt_14h
.type   interrupt_14h, @function
interrupt_14h:
    push ds
    push es
    push bp
    push di

    mov  di, 0xF000
    mov  ds, di

    cmp  ah, 0x04
    jae  interrupt_14h_end

    push dx
    push cx
    push bx
    push ax

    mov  bp, sp
    push bp
    call serial_handler
    pop  bp

    pop  ax
    pop  bx
    pop  cx
    pop  dx

interrupt_14h_end:
    pop  di
    pop  bp
    pop  es
    pop  ds
    iret