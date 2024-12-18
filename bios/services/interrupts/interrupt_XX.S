.code16
.arch i8086

.section .legacy.intirq, "ax", @progbits
.global interrupt_irq
.type   interrupt_irq, @function
interrupt_irq:
    cli
    push %ax
    mov  $0x20, %al
    out  %al, $0x20
    pop  %ax
    iret

.section .text.interrupt_XXh, "ax", @progbits
.global interrupt_XXh
.type   interrupt_XXh, @function
interrupt_XXh:
.global interrupt_1Bh
.type   interrupt_1Bh, @function
interrupt_1Bh:
.global interrupt_1Ch
.type   interrupt_1Ch, @function
interrupt_1Ch:
    iret

