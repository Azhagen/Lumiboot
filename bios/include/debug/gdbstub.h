#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "attrib.h"

#define GDB_SUCCESS 0x00
#define GDB_ERROR   0x01
#define GDB_EOF     0xFF

#define GDB_X86_REGISTER_COUNT 16

typedef struct dbg_registers dbg_registers_t;
typedef struct gdb_state gdb_state_t;

void gdb_init(void);

void trap_interrupt(dbg_registers_t __seg_ss* regs);
void uart_interrupt(dbg_registers_t __seg_ss* regs);