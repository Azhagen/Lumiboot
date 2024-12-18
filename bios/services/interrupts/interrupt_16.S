.code16
.arch i8086
.intel_syntax noprefix

#define BDA_SEG  0x40

.section .legacy.int16h, "ax", @progbits
.global interrupt_16h
.type   interrupt_16h, @function
interrupt_16h:
    sti
    push ds
    push di
    
    mov  di, 0xF000
    mov  ds, di

    pushf
    push dx
    push cx
    push bx
    push ax

    mov  di, sp
    push di

    call keyboard_handler

interrupt_16h_end:
    pop  di
    pop  ax
    pop  bx
    pop  cx
    pop  dx
    popf
    
    pop  di
    pop  ds
    retf 2

/*
kbd_read_key:
    # call asm_kbd_read_code
    call keyboard_read_key
    jmp  interrupt_16h_end

kbd_peek_key:
    # call asm_kbd_peek_code
    call keyboard_peek_key
    jmp  interrupt_16h_end

kbd_read_flag:
    jmp  interrupt_16h_end

kbd_store_key:
    jmp  interrupt_16h_end
*/