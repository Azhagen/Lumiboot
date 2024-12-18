#include "bios.h"
#include "attrib.h"
#include "io.h"
#include "debug.h"
#include "interrupt.h"
#include "utility.h"

// extern __regparmcall void int15h_call(intregs __seg_ss* const regs);
// extern void intcall(bioscall __seg_ss* const regs, uint16_t func);

static void int15h_call(bioscall __seg_ss* const regs)
{
    asm volatile (
        "int $0x15\n\r"
        : "+a"(regs->ax)
        :: "memory", "cc", "si", "di", "es", "ds");
}

uint8_t bios_system_kbd_intercept(uint8_t code)
{
    bioscall regs = {0};
    regs.ah = 0x00;
    regs.al = code;
    int15h_call(&regs);
    return regs.al;
}
