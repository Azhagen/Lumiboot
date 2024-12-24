.code16
.intel_syntax noprefix

#define PIC_CMD1 0x20
#define FDC_CTRL 0x3F2

#define BDA_SEG  0x40   // BDA segment

#define BDA_MTST 0x3F   // Motor status
#define BDA_MTSO 0x40   // Floppy motor shutoff
#define BDA_TILO 0x6C   // Time of day low
#define BDA_TIHI 0x6E   // Time of day high
#define BDA_TIOF 0x70   // Time of day overflow

#define EOI 0x20

#define LO 0
#define HI 2
#define OF 4

.section .legacy.int08h, "ax", @progbits
.global interrupt_08h
.type   interrupt_08h, @function
interrupt_08h:
    push ds
    push ax
    push dx

    mov  ax, BDA_SEG
    mov  ds, ax

    addw [BDA_TILO], 1
    adcw [BDA_TIHI], 0
    cmpw [BDA_TIHI], 0x18
    jb   fdc_timer_dec

    cmpw [BDA_TILO], 0xB0
    jb   fdc_timer_dec

pit_timer_rst:
    movw [BDA_TILO], 0
    movw [BDA_TIHI], 0
    incw [BDA_TIOF]

fdc_timer_dec:
    decb [BDA_MTSO]
    jnz  interrupt_08h_end

fdc_timer_act:
    andb [BDA_MTST], 0xF0
    mov  al, 0x0C
    mov  dx, FDC_CTRL
    out  dx, al

interrupt_08h_end:
    int  0x1C

    mov  al, EOI
    out  PIC_CMD1, al

    pop  dx
    pop  ax
    pop  ds
    iret