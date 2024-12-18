#pragma once

#include "services/block.h"

#include <stdint.h>
#include "attrib.h"
#include "interrupt.h"



// void floppy_system_status(bioscall __seg_ss* const regs);
// void floppy_drive_parameters(bioscall __seg_ss* const regs);
// void floppy_drive_type(bioscall __seg_ss* const regs);
// void floppy_detect_change(bioscall __seg_ss* const regs);


void floppy_reset(registers_t __seg_ss* const regs);
void floppy_status(registers_t __seg_ss* const regs);
void floppy_read(registers_t __seg_ss* const regs);
void floppy_write(registers_t __seg_ss* const regs);
void floppy_verify(registers_t __seg_ss* const regs);
void floppy_format(registers_t __seg_ss* const regs);

// void floppy_format_cylinder(bioscall __seg_ss* const regs);
// void floppy_format_cylinder(bioscall __seg_ss* const regs);