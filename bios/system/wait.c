#include "system/wait.h"
#include "utility.h"
#include "system/data.h"

#include "debug.h"

void pit_wait(uint32_t msecs)
{
    uint32_t off = 1821L * msecs / 100000L;
    uint32_t clk = as_uint32(bda->timer_hi, bda->timer_lo);
    uint32_t end = clk + off;

    while (clk < end)
    {
        clk = as_uint32(bda->timer_hi, bda->timer_lo);
        asm volatile ("hlt" ::: "memory");
    }
}

bool pit_wait_until(uint32_t msecs, bool (*function)(void))
{
    // debug_out("msecs = %lu\r\n", msecs);

    uint32_t off = 1821L * msecs / 100000L;
    uint32_t clk = as_uint32(bda->timer_hi, bda->timer_lo);
    uint32_t end = clk + off;

    // debug_out("clk = %lx, end = %lx\r\n", clk, end);
    // debug_out("off = %lx, msecs = %lu\r\n", off, msecs);

    while (!function())
    {
        clk = as_uint32(bda->timer_hi, bda->timer_lo);
        if (clk >= end) return false;
        asm volatile ("hlt" ::: "memory");
    }

    // debug_out("return true\r\n");
    return true;
}