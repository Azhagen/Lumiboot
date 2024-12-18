.code16
.intel_syntax noprefix

.section .reset, "ax", @progbits
.global reset
.type   reset, @function
reset:
    jmp 0xF000:entry16

.global date
.type   date, @object
date:
    .asciz "01/01/24"

.global archid
.type   archid, @object
archid:
    .byte 0xFC

.global checksum
.type   checksum, @object
checksum:
    .byte 0x00