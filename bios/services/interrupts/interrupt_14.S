.code16
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
    pusha
    push es
    push ds
    pushf

    mov  di, 0xF000
    mov  ds, di

    mov  bp, sp
    push bp
    call serial_handler
    pop  bp

    popf
    pop  ds
    pop  es
    popa
    iret