#include "bios.h"
#include "attrib.h"
#include "io.h"
#include "debug.h"
#include "interrupt.h"
#include "utility.h"

extern void intcall(intregs __seg_ss* const regs, uint16_t func);

void bios_bootstrap(void)
{
    asm volatile ("int $0x19" ::: "memory");
    asm volatile ("int $0x18" ::: "memory");
}