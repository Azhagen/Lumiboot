#include <stdint.h>
#include <stddef.h>
#include "string.h"
#include "attrib.h"
#include "utility.h"

void *memcpy(void *restrict dst,
    const void *restrict src, size_t cnt)
{
    uint8_t* dst0 = dst;
    const uint8_t* src0 = src;

    for (size_t i = 0; i < cnt; ++i)
        dst0[i] = src0[i];

    return dst;
}

void* memset(void *restrict ptr, int value, size_t num)
{
    uint8_t* ptr0 = ptr;
    for (size_t i = 0; i < num; i++)
        ptr0[i] = (uint8_t)value;

    return ptr;
}

size_t strlen(const char* str)
{
    size_t len = 0;
    while (*str++ != 0)
        len++;
    return len;
}

void __far* fmemcpy_8(void __far* restrict dst,
    const void __far* restrict src, size_t cnt)
{
    pointer dst0 = (pointer)dst;
    pointer src0 = (pointer)(void __far*)src;

    asm volatile (
        "mov %0, %%es\n\t"
        "mov %1, %%ds\n\t"
        "cld\n\t"
        "rep movsb\n\t"
        :: "r"(dst0.seg), "r"(src0.seg), "D"(dst0.off), "S"(src0.off), "c"(cnt)
        : "memory", "cc", "es", "ds"
    );

    return dst;
}

void __far* fmemset_8(void __far* restrict dst, uint8_t val, size_t cnt)
{
    pointer dst0 = (pointer)dst;

    asm volatile (
        "mov %0, %%es\n\t"
        "cld\n\t"
        "rep stosb\n\t"
        :: "r"(dst0.seg), "D"(dst0.off), "a"(val), "c"(cnt)
        : "memory", "cc", "es"
    );

    return dst;
}

void __far* fmemcpy_16(void __far* restrict dst,
    const void __far* restrict src, size_t cnt)
{
    pointer dst0 = (pointer)dst;
    pointer src0 = (pointer)(void __far*)src;

    asm volatile (
        "mov %0, %%es\n\t"
        "mov %1, %%ds\n\t"
        "cld\n\t"
        "rep movsw\n\t"
        :: "r"(dst0.seg), "r"(src0.seg), "D"(dst0.off), "S"(src0.off), "c"(cnt)
        : "memory", "cc", "es", "ds"
    );

    return dst;
}

void __far* fmemset_16(void __far* restrict dst, uint16_t val, size_t cnt)
{
    pointer dst0 = (pointer)dst;

    asm volatile (
        "mov %0, %%es\n\t"
        "cld\n\t"
        "rep stosw\n\t"
        :: "r"(dst0.seg), "D"(dst0.off), "a"(val), "c"(cnt)
        : "memory", "cc", "es"
    );

    return dst;
}

size_t fstrlen(const char __far* str)
{
    size_t len = 0;
    while (*str++ != 0)
        len++;
    return len;
}