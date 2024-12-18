#pragma once

#include <stdint.h>
#include <stdbool.h>

#include "attrib.h"
#include "interrupt.h"

typedef struct
{
    WORD(a);
    WORD(c);
    WORD(d);
    FLAGS;
} time_regs;

void time_read_clock(time_regs __seg_ss* const regs);
void time_write_clock(time_regs __seg_ss* const regs);
void time_read_time(time_regs __seg_ss* const regs);
void time_write_time(time_regs __seg_ss* const regs);
void time_read_date(time_regs __seg_ss* const regs);
void time_write_date(time_regs __seg_ss* const regs);
void time_set_alarm(time_regs __seg_ss* const regs);
void time_reset_alarm(time_regs __seg_ss* const regs);