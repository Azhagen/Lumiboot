#pragma once

#include <stdint.h>
#include <stddef.h>
#include "attrib.h"

struct __packed gdt_entry
{
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t base_middle;
    uint8_t access;
    uint8_t granularity;
    uint8_t base_high;
};

struct __packed gdt_ptr
{
    uint16_t limit;
    uint32_t base;
};

struct __packed idt_ptr
{
    uint16_t limit;
    uint32_t base;
};

typedef struct gdt_entry gdt_entry_t;
typedef struct gdt_ptr   gdt_ptr_t;
typedef struct idt_ptr   idt_ptr_t;