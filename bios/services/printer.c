#include "services/printer.h"
#include "drivers.h"
#include "debug.h"

void printer_handler(registers_t __seg_ss* const regs)
{
    switch (regs->ah)
    {
        case 0x00: printer_send(regs); break;
        case 0x01: printer_init(regs); break;
        case 0x02: printer_status(regs); break;
        default: break;
    }
}

void printer_send(registers_t __seg_ss* const regs)
{
    regs->ah = lpt_print(regs->dx, regs->al);
}

void printer_init(registers_t __seg_ss* const regs)
{
    regs->ah = lpt_initialize(regs->dx);
}

void printer_status(registers_t __seg_ss* const regs)
{
    regs->ah = lpt_read_status(regs->dx);
}