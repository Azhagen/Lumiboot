#pragma once

#include "interrupt.h"

// INT 15H

void system_kbd_intercept(bioscall __seg_ss* const regs);

void system_joy_handler(bioscall __seg_ss* const regs);

void system_async_wait(bioscall __seg_ss* const regs);
void system_sync_wait(bioscall __seg_ss* const regs);

void system_move_blocks(bioscall __seg_ss* const regs);
void system_memory_size(bioscall __seg_ss* const regs);
void system_enter_pmode(bioscall __seg_ss* const regs);
void system_read_config(bioscall __seg_ss* const regs);

void system_request(bioscall __seg_ss* const regs);
void system_device_open(bioscall __seg_ss* const regs);
void system_device_close(bioscall __seg_ss* const regs);
void system_device_busy(bioscall __seg_ss* const regs);
void system_program_end(bioscall __seg_ss* const regs);
void system_interrupt_end(bioscall __seg_ss* const regs);

// system_wait_async subfunctions
void sub_wait_interval_set(bioscall __seg_ss* const regs);
void sub_wait_interval_cancel(bioscall __seg_ss* const regs);

// system_read_joystick subfunctions
void sub_joystick_read_settings(bioscall __seg_ss* const regs);
void sub_joystick_read_inputs(bioscall __seg_ss* const regs);
