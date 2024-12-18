#pragma once

#include <stdint.h>
#include "attrib.h"
#include "interrupt.h"

// INT 16H

void keyboard_read_key(intregs __seg_ss* const regs);
void keyboard_peek_key(intregs __seg_ss* const regs);
void keyboard_get_flags(intregs __seg_ss* const regs);
void keyboard_set_bits(intregs __seg_ss* const regs);
void keyboard_store_key(intregs __seg_ss* const regs);

void keyboard_ext_read_key(intregs __seg_ss* const regs);
void keyboard_ext_peek_key(intregs __seg_ss* const regs);
void keyboard_ext_get_flags(intregs __seg_ss* const regs);