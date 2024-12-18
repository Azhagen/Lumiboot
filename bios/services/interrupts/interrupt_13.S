.code16
.intel_syntax noprefix

.section .legacy.int13h, "ax", @progbits

.global interrupt_13h
.type   interrupt_13h, @function
interrupt_13h:
    sti

    pushf
    push ds
    push es
    push si
    push di
    push dx
    push cx
    push bx
    push ax

    mov  ax, 0xF000
    mov  ds, ax
    mov  ax, sp
    push ax

    call block_handler

interrupt_13h_end:
    add  sp, 2

    pop  ax
    pop  bx
    pop  cx
    pop  dx
    pop  di
    pop  si
    pop  es
    pop  ds
    popf

    retf 2

/*
.type asm_floppy_reset, @function
asm_floppy_reset:
    call floppy_reset
    jmp  interrupt_13h_end

.type asm_floppy_status, @function
asm_floppy_status:
    call floppy_status
    jmp  interrupt_13h_end

.type asm_floppy_read, @function
asm_floppy_read:
    call floppy_read
    jmp  interrupt_13h_end

.type asm_floppy_write, @function
asm_floppy_write:
    call floppy_write
    jmp  interrupt_13h_end

.type asm_floppy_verify, @function
asm_floppy_verify:
    call floppy_verify
    jmp  interrupt_13h_end

.type asm_floppy_format, @function
asm_floppy_format:
    call floppy_format
    jmp  interrupt_13h_end
*/