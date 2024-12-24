#pragma once

#include <stdint.h>
#include <stdbool.h>

#include "attrib.h"
#include "interrupt.h"

void time_read_clock(registers_t __seg_ss* const regs);
void time_write_clock(registers_t __seg_ss* const regs);
void time_read_time(registers_t __seg_ss* const regs);
void time_write_time(registers_t __seg_ss* const regs);
void time_read_date(registers_t __seg_ss* const regs);
void time_write_date(registers_t __seg_ss* const regs);
void time_set_alarm(registers_t __seg_ss* const regs);
void time_reset_alarm(registers_t __seg_ss* const regs);