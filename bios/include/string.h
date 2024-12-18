#pragma once

#include <stddef.h>
#include "attrib.h"

void* memcpy(void *restrict dst, const void *restrict src, size_t cnt);
void* memset(void *restrict ptr, int value, size_t num);
size_t strlen(const char* str);

void __far* fmemcpy_8(void __far* restrict dst, const void __far* restrict src, size_t cnt);
void __far* fmemset_8(void __far* restrict dst, uint8_t val, size_t cnt);
void __far* fmemcpy_16(void __far* restrict dst, const void __far* restrict src, size_t cnt);
void __far* fmemset_16(void __far* restrict dst, uint16_t val, size_t cnt);
size_t fstrlen(const char __far* str);