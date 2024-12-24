#pragma once

#include <stdint.h>
#include "attrib.h"
#include "interrupt.h"

// INT 17

void printer_send(registers_t __seg_ss* const regs);
void printer_init(registers_t __seg_ss* const regs);
void printer_status(registers_t __seg_ss* const regs);