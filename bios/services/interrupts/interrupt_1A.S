.code16
.intel_syntax noprefix

#define BDA_SEGM 0x40   // BDA segment
#define BDA_TILO 0x6C   // Time of day low
#define BDA_TIHI 0x6E   // Time of day high
#define BDA_TIOF 0x70   // Time of day overflow

.section .legacy.int1Ah, "ax", @progbits
.global interrupt_1Ah
.type   interrupt_1Ah, @function
interrupt_1Ah:
    push ds
    push si

    mov  si, 0xF000
    mov  ds, si

    pushf
    push dx
    push cx
    push ax

    mov  si, sp
    push si

    call time_handler

/*
    cmp  ah, 0
    je   asm_time_read_sysclock
    cmp  ah, 1
    je   asm_time_write_sysclock
*/
interrupt_1Ah_end:
    pop  si
    pop  ax
    pop  cx
    pop  dx
    popf
    
    pop  si
    pop  ds
    retf 2

/*
asm_time_read_sysclock:
    call time_read_sysclock
    jmp  interrupt_1Ah_end

asm_time_write_sysclock:
    call time_write_sysclock
    jmp  interrupt_1Ah_end
*/