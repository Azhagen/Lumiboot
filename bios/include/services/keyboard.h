#pragma once

#include <stdint.h>
#include "attrib.h"
#include "interrupt.h"

// INT 16H

void keyboard_read_key(registers_t __seg_ss* const regs);
void keyboard_peek_key(registers_t __seg_ss* const regs);
void keyboard_get_flags(registers_t __seg_ss* const regs);
void keyboard_set_bits(registers_t __seg_ss* const regs);
void keyboard_store_key(registers_t __seg_ss* const regs);

void keyboard_ext_read_key(registers_t __seg_ss* const regs);
void keyboard_ext_peek_key(registers_t __seg_ss* const regs);
void keyboard_ext_get_flags(registers_t __seg_ss* const regs);