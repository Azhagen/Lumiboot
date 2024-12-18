#include "services/printer.h"
#include "drivers.h"

// void printer_send(bioscall __seg_ss* const regs)
// {
//     regs->ah = lpt_print(regs->dx, regs->al);
// }

// void printer_init(bioscall __seg_ss* const regs)
// {
//     regs->ah = lpt_initialize(regs->dx);
// }

// void printer_status(bioscall __seg_ss* const regs)
// {
//     regs->ah = lpt_read_status(regs->dx);
// }