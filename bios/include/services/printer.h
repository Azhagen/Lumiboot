#pragma once

#include <stdint.h>
#include "attrib.h"
#include "interrupt.h"

// INT 17

void printer_send(bioscall __seg_ss* const regs);
void printer_init(bioscall __seg_ss* const regs);
void printer_status(bioscall __seg_ss* const regs);