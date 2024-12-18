#include "bios.h"
#include "attrib.h"
#include "io.h"
#include "debug.h"
#include "interrupt.h"
#include "utility.h"

// extern __regparmcall void int15h_call(intregs __seg_ss* const regs);
// extern void intcall(bioscall __seg_ss* const regs, uint16_t func);

static void int16h_call(bioscall __seg_ss* const regs)
{
    asm volatile (
        "int $0x16\n\r"
        : "+a"(regs->ax)
        : "b"(regs->bx)
        : "memory", "cc");
}

uint16_t bios_keyboard_read_key(void)
{
    bioscall regs = {0};
    int16h_call(&regs);
    return regs.ax;
}

uint16_t bios_keyboard_peek_key(void)
{
    bioscall regs = {0};
    regs.ah = 0x01;
    int16h_call(&regs);
    return regs.ax;
}