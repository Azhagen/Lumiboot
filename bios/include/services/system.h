#pragma once

#include "interrupt.h"

// INT 15H

void system_kbd_intercept(registers_t __seg_ss* const regs);

void system_joy_handler(registers_t __seg_ss* const regs);

void system_async_wait(registers_t __seg_ss* const regs);
void system_sync_wait(registers_t __seg_ss* const regs);

void system_move_blocks(registers_t __seg_ss* const regs);
void system_memory_size(registers_t __seg_ss* const regs);
void system_enter_pmode(registers_t __seg_ss* const regs);
void system_read_config(registers_t __seg_ss* const regs);

void system_request(registers_t __seg_ss* const regs);
void system_device_open(registers_t __seg_ss* const regs);
void system_device_close(registers_t __seg_ss* const regs);
void system_device_busy(registers_t __seg_ss* const regs);
void system_program_end(registers_t __seg_ss* const regs);
void system_interrupt_end(registers_t __seg_ss* const regs);

// system_wait_async subfunctions
void sub_wait_interval_set(registers_t __seg_ss* const regs);
void sub_wait_interval_cancel(registers_t __seg_ss* const regs);

// system_read_joystick subfunctions
void sub_joystick_read_settings(registers_t __seg_ss* const regs);
void sub_joystick_read_inputs(registers_t __seg_ss* const regs);
