#pragma once

#include <stdint.h>
#include "attrib.h"
#include "interrupt.h"

void block_reset(registers_t __seg_ss* const regs);
void block_status(registers_t __seg_ss* const regs);
void block_read(registers_t __seg_ss* const regs);
void block_write(registers_t __seg_ss* const regs);
void block_verify(registers_t __seg_ss* const regs);
void block_format(registers_t __seg_ss* const regs);
void block_params(registers_t __seg_ss* const regs);
void block_type(registers_t __seg_ss* const regs);
void block_change(registers_t __seg_ss* const regs);

void block_ext_check(registers_t __seg_ss* const regs);
void block_ext_read(registers_t __seg_ss* const regs);
void block_ext_write(registers_t __seg_ss* const regs);
void block_ext_verify(registers_t __seg_ss* const regs);
void block_ext_locking(registers_t __seg_ss* const regs);
void block_ext_dskchg(registers_t __seg_ss* const regs);
void block_ext_seek(registers_t __seg_ss* const regs);
void block_ext_params(registers_t __seg_ss* const regs);
void block_ext_status(registers_t __seg_ss* const regs);