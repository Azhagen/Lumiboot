.code16

.section .text.boot_start, "axR", @progbits
.global boot_start
.type   boot_start, @function
boot_start:
    push %bp
    mov  %sp, %bp
    mov  4(%bp), %dl
    ljmp $0x0000, $0x7C00

loop:
    jmp  loop