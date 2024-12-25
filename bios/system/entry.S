.code16
.intel_syntax noprefix

#define BDA_SEGM 0x40
#define BDA_RPTR 0x67 /* return pointer */
#define BDA_SPTR 0xAC /* return stack */

.section .text.entry16, "ax", @progbits
.global entry16
.type   entry16, @function
entry16:
    cli
    cld

    // Check the reset code
    mov  al, 0x80 | 0x0F
    out  0x70, al
    in   al, 0x71
    cmp  al, 0x0A
    je   entry_rmode

    // Set the timer for memory refresh
    mov  al, 0x64
    out  0x43, al
    mov  ax, 2386
    out  0x41, al
    mov  al, ah
    out  0x41, al

    mov  ax, 0xF000
    mov  ds, ax
    mov  sp, 0x0400
    call bios_init

    jmp  $

entry_rmode:
    mov  ax, BDA_SEGM
    mov  ds, ax

    mov  al, 0x80 | 0x0F
    out  0x70, al
    mov  al, 0x00
    out  0x71, al

    mov  ss, [BDA_SPTR + 0]
    mov  sp, [BDA_SPTR + 2]

    push [BDA_RPTR + 0]
    push [BDA_RPTR + 2]
    retf