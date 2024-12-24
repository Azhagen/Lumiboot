#pragma once

#include <stdint.h>
#include "attrib.h"

#include "debug/gdbstub.h"

enum debug_device
{
    DBG_COM1,
    DBG_COM2,
    DBG_COM3,
    DBG_COM4,
    DBG_0xE9,
    DBG_MDA,
};

void debug_init(void);

void debug_putc(uint8_t ch);
void debug_puts(const char __far* str);
void debug_putx(uint32_t value);

void debug_set_output(uint8_t device);

void debug_out(const char __far* str, ...);