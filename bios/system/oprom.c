#include "utility.h"
#include "debug.h"
#include "io.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct __packed
{
    uint16_t magic;
    uint8_t  size;
    uint8_t  code[];
} option_rom;

typedef void __far (*function)(void);

__noinline
static void farcall(function func)
{
    asm volatile (
        "push %%ds\n\t"
        "push %%es\n\t"
        "push %%si\n\t"
        "push %%di\n\t"
        "push %%bp\n\t"
        "pushf\n\t"
        "lcall *%0\n\t"
        "popf\n\t"
        "pop  %%bp\n\t"
        "pop  %%di\n\t"
        "pop  %%si\n\t"
        "pop  %%es\n\t"
        "pop  %%ds\n\t"
        :: "m"(func) : "memory");
}

static bool compute_rom_checksum(void __far* const ptr, size_t size)
{
    uint8_t __far* data = ptr;
    uint8_t count = 0;
    
    for (size_t i = 0; i < size; ++i)
        count = (uint8_t)(count + data[i]);

    return count == 0;
}

static void rom_do_init(uint32_t begin, uint32_t end)
{
    for (uint32_t linear = begin; linear < end; linear += 0x800)
    {
        option_rom __far* rom = linear_to_fp(linear);

        if (rom->magic != 0xAA55)
            continue;

        debug_out("[BIOS] Found ROM: %lp\n\r", linear);

        if (!compute_rom_checksum(rom, (size_t)(rom->size * 512)))
            continue;

        debug_out("[BIOS] ROM checksum OK\n\r");

        farcall((void __far*)(&rom->code));
    }
}

void rom_init(void)
{
    rom_do_init(0xC0000, 0xF0000);
}

// void rom_init_early(void)
// {
//     rom_do_init(0xC0000, 0xC8000);
// }