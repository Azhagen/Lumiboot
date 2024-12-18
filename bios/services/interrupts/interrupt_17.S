.code16
.arch i8086
.intel_syntax noprefix

#define BDA_SEG  0x40
#define BDA_LPT  0x08

#define BDA_TLPT 0x78   // Timeout LPT
#define BDA_TILO 0x6C   // Time of day low

.section .legacy.int17h, "ax", @progbits

.global interrupt_17h
.type   interrupt_17h, @function
interrupt_17h:
    sti
    push ds
    push si

    mov  si, BDA_SEG
    mov  ds, si
    cmp  ah, 0
    je   printer_send
    cmp  ah, 1
    je   printer_init
    cmp  ah, 2
    je   printer_stat

interrupt_17h_end:
    pop  si
    pop  ds
    iret

.global printer_send
.type   printer_send, @function
printer_send:
    jmp  interrupt_17h_end

.global printer_init
.type   printer_init, @function
printer_init:
    jmp  interrupt_17h_end

.global printer_stat
.type   printer_stat, @function
printer_stat:
    jmp  interrupt_17h_end