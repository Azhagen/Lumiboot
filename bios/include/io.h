#pragma once

#include <stdint.h>
#include <stddef.h>
#include "attrib.h"
#include "utility.h"

static inline void io_write(uint16_t port, uint8_t value)
{
    asm volatile ("out %0, %1" :: "Ral"(value), "Nd"(port) : "memory");
}

static inline uint8_t io_read(uint16_t port)
{
    uint8_t value;
    asm volatile ("inb %1, %0" : "=Ral"(value) : "Nd"(port) : "memory");
    return value;
}

static inline void io_write16(uint16_t port, uint16_t value)
{
    asm volatile ("outw %0, %1" :: "a"(value), "Nd"(port) : "memory");
}

static inline uint16_t io_read16(uint16_t port)
{
    uint16_t value;
    asm volatile ("inw %1, %0" : "=a"(value) : "Nd"(port) : "memory");
    return value;
}

static inline void io_wait(uint32_t ticks)
{
    for (uint32_t i = 0; i < ticks; ++i)
        io_read(0x80);
}