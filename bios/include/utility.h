#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include "attrib.h"

#define CONST(x) __builtin_constant_p(x) ? (x) : (x)

typedef void __far* farptr_t;

typedef union __attribute__((__transparent_union__)) pointer
{
    struct
    { 
        uint16_t off;
        uint16_t seg;
    };
    void __far* ptr;
    void __far* restrict resptr;
    uint32_t value;
    void __far (*func)(void);
} pointer;

static_assert(sizeof(pointer) == 4, "pointer size must be 4 bytes");

#define array_size(x) sizeof(x) / sizeof(x[0])

static inline uint32_t as_linear(pointer ptr)
{
    return (uint32_t)(ptr.seg) * 16 + ptr.off;
}

static inline uint32_t to_linear(void __far *addr)
{
    pointer ptr = (pointer)addr;
    return (uint32_t)(ptr.seg) * 16 + ptr.off;
}

/*
static inline void __far *linear_to_segoff(uint32_t addr)
{
    pointer ptr = {0};
    ptr.seg = (uint16_t)(addr % 0x10);
    ptr.off = (uint16_t)(addr / 0x10);
    return ptr.ptr;
}
*/

static inline uint8_t hi(uint16_t value)
{
    return (uint8_t)((value & 0xFF00) >> 8);
}

static inline uint8_t lo(uint16_t value)
{
    return (uint8_t)(value & 0xFF);
}

static inline uint16_t lo16(uint32_t value)
{
    return (uint16_t)(value & 0xFFFF);
}

static inline uint16_t hi16(uint32_t value)
{
    return (uint16_t)((value & 0xFFFF0000) >> 16);
}

static inline farptr_t segoff_to_fp(uint16_t seg, uint16_t off)
{
    pointer ptr;
    ptr.seg = seg;
    ptr.off = off;
    return ptr.ptr;
}

static inline pointer linear_to_ptr(uint32_t addr)
{
    pointer ptr;
    ptr.seg = (uint16_t)((addr >> 4) & 0xF000);
    ptr.off = (uint16_t)(addr & 0xFFFF);
    return ptr;
}

static inline farptr_t linear_to_fp(uint32_t addr)
{
    pointer ptr;
    ptr.seg = (uint16_t)((addr & 0xF0000) >> 4);
    ptr.off = (uint16_t)(addr & 0xFFFF);
    return ptr.ptr;
}

static inline uint32_t segoff_to_linear(uint16_t seg, uint16_t off)
{
    return ((uint32_t)(seg) << 4) + off;
}

static inline uint16_t linear_to_seg(uint32_t addr)
{
    return (uint16_t)(addr >> 4);
}

static inline uint16_t as_uint16(uint8_t hi, uint8_t lo)
{
    return ((uint16_t)(hi) << 8) | lo;
}

static inline uint32_t as_uint32(uint16_t hi, uint16_t lo)
{
    return ((uint32_t)hi << 16) | (uint32_t)lo;
}

static inline void __far* to_fp(void* addr)
{
    return __builtin_ia16_static_far_cast(addr);
}

static inline void __far* ss_to_fp(void __seg_ss* addr)
{
    return addr;
}

static inline uint8_t binary_to_bcd(uint8_t value)
{
    return (uint8_t)((value / 10 * 16) + (value % 10));
}

static inline uint8_t bcd_to_binary(uint8_t value)
{
    return (uint8_t)((value / 16 * 10) + (value % 16));
}

static inline uint32_t to_le32(uint32_t value)
{
    return (uint32_t)((value & 0xFF) << 24) |
           (uint32_t)((value & 0xFF00) << 8) |
           (uint32_t)((value & 0xFF0000) >> 8) |
           (uint32_t)((value & 0xFF000000) >> 24);
}