#pragma once

#include "interrupt.h"

void serial_initialize(intregs __seg_ss* const regs);
void serial_status(intregs __seg_ss* const regs);
void serial_transmit(intregs __seg_ss* const regs);
void serial_receive(intregs __seg_ss* const regs);
