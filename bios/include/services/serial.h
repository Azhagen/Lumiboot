#pragma once

#include "interrupt.h"

void serial_initialize(registers_t __seg_ss* const regs);
void serial_status(registers_t __seg_ss* const regs);
void serial_transmit(registers_t __seg_ss* const regs);
void serial_receive(registers_t __seg_ss* const regs);
