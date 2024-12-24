.code16
.intel_syntax noprefix

.section .text.boot_start, "ax", @progbits
.global boot_start
.type   boot_start, @function
boot_start:
    push bp
    mov  bp, sp
    mov  dl, [bp + 4]
    jmp  0x0:0x7C00

    jmp  $