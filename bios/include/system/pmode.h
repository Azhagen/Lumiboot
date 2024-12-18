#pragma once

#include <stdint.h>

void pmode_memcpy(gdt_ptr_t __far* gdtptr, uint16_t count);
