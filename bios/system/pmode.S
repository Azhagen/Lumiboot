.code16
.intel_syntax noprefix

#define BDA_SEGM 0x40
#define BDA_RPTR 0x67 /* return pointer */
#define BDA_SPTR 0xAC /* return stack */

.section .text.pmode_execute, "ax", @progbits
.global pmode_execute
.type   pmode_execute, @function
pmode_execute:
    push bp
    mov  bp, sp
    push di
    push si
    push es
    push ds

    mov  ax, BDA_SEGM
    mov  ds, ax

    movw [BDA_RPTR + 0], 0xF000
    movw [BDA_RPTR + 2], OFFSET pmode_execute_rmode
    movw [BDA_SPTR + 0], ss
    movw [BDA_SPTR + 2], sp
    subw [BDA_SPTR + 2], 2

    smsw ax
    or   ax, 1
    lmsw ax

    jmp  0x08:pmode_execute_pmode

pmode_execute_pmode:
    mov  ax, 0x10
    mov  ds, ax
    mov  ss, ax
    mov  es, ax

    call [bp + 4]
    push ax // save return value

    mov  al, 0x80 | 0x0F
    out  0x70, al
    mov  al, 0x0A
    out  0x71, al

    mov  eax, cr0
    and  eax, 0x7FFFFFFE
    mov  cr0, eax

    jmp 0xF000:pmode_execute_rmode_386

pmode_execute_rmode_386:
    mov  ax, BDA_SEGM
    mov  ds, ax

    mov  ss, [BDA_SPTR + 0]
    mov  sp, [BDA_SPTR + 2]

pmode_execute_rmode:
    pop  ax
    pop  ds
    pop  es
    pop  si
    pop  di
    pop  bp
    ret

.section .text.pmode_memcpy, "ax", @progbits
.global pmode_memcpy
.type   pmode_memcpy, @function
pmode_memcpy:
    push bp
    mov  bp, sp
    cli

    pusha
    push es
    push ds

    mov  ax, BDA_SEGM
    mov  ds, ax

    mov  [BDA_RPTR + 0], cs
    movw [BDA_RPTR + 2], OFFSET pmode_memcpy_rmode
    mov  [BDA_SPTR + 0], ss
    mov  [BDA_SPTR + 2], sp

    mov  bx, [bp + 4]
    mov  es, [bp + 6]

    lgdt es:[bx]

    mov  cx, [bp + 8]

    smsw ax
    or   ax, 1
    lmsw ax
    
    jmp  0x08:pmode_memcpy_pmode

pmode_memcpy_pmode:
    mov  ax, 0x10
    mov  ds, ax
    mov  ax, 0x18
    mov  es, ax
    mov  si, 0
    mov  di, 0
    cld
    rep  movsw /* ds:si -> es:di */

    mov  al, 0x80 | 0x0F
    out  0x70, al
    mov  al, 0x0A
    out  0x71, al

    mov  eax, cr0
    and  eax, 0xFFFFFFFE
    mov  cr0, eax

    jmp 0xF000:pmode_memcpy_rmode

pmode_memcpy_rmode:
    pop  ds
    pop  es
    popa

    sti
    pop  bp
    ret