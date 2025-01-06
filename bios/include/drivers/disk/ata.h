#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "attrib.h"
#include "system/block.h"

// ATA-1 Specification
struct __packed ata_identity
{
    uint16_t flags;
    uint16_t cylinders;
    uint16_t __reserved0;
    uint16_t heads;
    uint16_t __reserved1;
    uint16_t __reserved2;
    uint16_t sectors;
    uint16_t __reserved3[3];
    uint8_t  serial[20];
    uint16_t __reserved4[3];
    uint8_t  firmware[8];
    uint8_t  model[40];
    uint16_t sectors_per_int;
    uint16_t __reserved5;
    uint16_t capabilities;
    uint16_t __reserved6;
    uint16_t pio_modes;
    uint16_t dma_modes;
    uint16_t __reserved7;
    uint16_t cur_cylinders;
    uint16_t cur_heads;
    uint16_t cur_sectors;
    uint32_t cur_capacity;
    uint16_t __reserved8;
    uint32_t lba_capacity;
    uint16_t dma_singleword;
    uint16_t dma_multiword;
    uint16_t __reserved9[191];
};

typedef struct ata_identity ata_identity_t;

void ata_detect(void);

uint8_t ata_send_cmd(block_t __far* blk, command_t __seg_ss* cmd);

uint8_t ata_reset(block_t __far* blk);

uint8_t ata_read_chs(block_t __far* blk, uint16_t cylinder, uint16_t head,
    uint16_t sector, uint8_t count, uint32_t buffer);
uint8_t ata_write_chs(block_t __far* blk, uint16_t cylinder, uint16_t head,
    uint16_t sector, uint8_t count, uint32_t buffer);
uint8_t ata_verify_chs(block_t __far* blk, uint16_t cylinder, uint16_t head,
    uint16_t sector, uint8_t count);
uint8_t ata_format_chs(block_t __far* blk, uint16_t cylinder, uint16_t head,
    uint16_t sector, uint8_t count, uint32_t buffer);

uint8_t ata_read_lba(block_t __far* blk, uint32_t lba, uint32_t count, uint32_t buffer);
uint8_t ata_write_lba(block_t __far* blk, uint32_t lba, uint32_t count, uint32_t buffer);
uint8_t ata_verify_lba(block_t __far* blk, uint32_t lba, uint32_t count);
uint8_t ata_seek_lba(block_t __far* blk, uint32_t lba);