.code16
.intel_syntax noprefix

.section .legacy.int1Ah, "ax", @progbits
.global interrupt_1Ah
.type   interrupt_1Ah, @function
interrupt_1Ah:
    pusha
    push es
    push ds
    pushf

    mov  si, 0xF000
    mov  ds, si

    mov  si, sp
    push si
    call time_handler
    pop  si

    popf
    pop  ds
    pop  es
    popa

    retf 2