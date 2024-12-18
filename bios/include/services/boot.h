#pragma once

#include <stdint.h>
#include "attrib.h"

typedef union
{
    struct 
    {
        uint8_t data[510];
        uint16_t signature;
    };
    uint8_t raw[512];
} boot_sector;

void boot_floppy(uint8_t drive);
void boot_disk(uint8_t drive);
void boot_no_device(void);