#include "system/boot.h"
#include "system/block.h"

#include "debug.h"
#include <stddef.h>

static uint16_t __far* bootsect = (void __far*)0x7C00L;

void boot_device(uint8_t id)
{
    block_t __far* blk = 0;
    if (!block_find(id, &blk))
        return;

    command_t cmd = {};
    cmd.cmd = BLOCK_CMD_CHS_READ;
    cmd.chs = (geometry_t){ 0, 0, 1 };
    cmd.cnt = 1;
    cmd.buf = 0x7C00L;

    uint8_t result = blk->send_cmd(blk, &cmd);

    debug_out("Result: %x\n\r", result);

    if (result != BLOCK_SUCCESS)
        return;

    debug_out("[BIOS] Boot: Boot sector read from device %u\n\r", id);

    for (size_t i = 0; i < 512; i += 16)
    {
        debug_out("[BIOS] Boot: %04X: ", i);
        for (size_t j = 0; j < 16; ++j)
            debug_out("%02X ", ((uint8_t __far*)bootsect)[i + j]);
        debug_puts("\n\r");
    }

    if (bootsect[510 / 2] != 0xAA55)
    {
        debug_out("[BIOS] Boot: Invalid boot sector signature: %04X\n\r", bootsect[510]);
        return;
    }


    boot_start(id);
}