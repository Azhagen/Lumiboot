#pragma once

#include <stdint.h>
#include <stddef.h>

#include "attrib.h"
#include "system/block.h"

void atapi_detect(void);
void __far atapi_boot(void);

uint8_t atapi_send_cmd(block_t __far* blk, command_t __seg_ss* cmd);

uint8_t atapi_read_lba(block_t __far* blk, uint32_t lba,
    uint32_t count, void __far* buffer);