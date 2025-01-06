#pragma once

#include "attrib.h"
#include "io.h"

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include "system/block.h"

// Internal floppy definitions

#define FLOPPY_DOR_MASK_DRIVE0       (0 << 0)
#define FLOPPY_DOR_MASK_DRIVE1       (1 << 0)
#define FLOPPY_DOR_MASK_DRIVE2       (2 << 0)
#define FLOPPY_DOR_MASK_DRIVE3       (3 << 0)
#define FLOPPY_DOR_MASK_RESET        (1 << 2)
#define FLOPPY_DOR_MASK_DMA          (1 << 3)
#define FLOPPY_DOR_MASK_DRIVE0_MOTOR (1 << 4)
#define FLOPPY_DOR_MASK_DRIVE1_MOTOR (1 << 5)
#define FLOPPY_DOR_MASK_DRIVE2_MOTOR (1 << 6)
#define FLOPPY_DOR_MASK_DRIVE3_MOTOR (1 << 7)

#define FLOPPY_MSR_MASK_DRIVE1_POS_MODE (1 << 0)
#define FLOPPY_MSR_MASK_DRIVE2_POS_MODE (1 << 1)
#define FLOPPY_MSR_MASK_DRIVE3_POS_MODE (1 << 2)
#define FLOPPY_MSR_MASK_DRIVE4_POS_MODE (1 << 3)
#define FLOPPY_MSR_MASK_BUSY			(1 << 4)
#define FLOPPY_MSR_MASK_DMA             (1 << 5)
#define FLOPPY_MSR_MASK_DATAIO          (1 << 6)
#define FLOPPY_MSR_MASK_DATAREG         (1 << 7)

#define FLOPPY_SECTORS_PER_TRACK 18

#define FDC_CMD_READ_TRACK   0x2
#define FDC_CMD_SPECIFY      0x3
#define FDC_CMD_CHECK_STAT   0x4
#define FDC_CMD_WRITE_SECT   0x5
#define FDC_CMD_READ_SECT    0x6
#define FDC_CMD_RECALIBRATE  0x7
#define FDC_CMD_CHECK_INT    0x8
#define FDC_CMD_WRITE_DEL_S  0x9
#define FDC_CMD_READ_ID_S    0xa
#define FDC_CMD_READ_DEL_S   0xc
#define FDC_CMD_FORMAT_TRACK 0xd
#define FDC_CMD_SEEK         0xf

#define FDC_CMD_EXT_SKIP        0x20
#define FDC_CMD_EXT_DENSITY     0x40
#define FDC_CMD_EXT_MULTITRACK  0x80

#define FLOPPY_GAP3_LENGTH_STD  42
#define FLOPPY_GAP3_LENGTH_5_14 32
#define FLOPPY_GAP3_LENGTH_3_5  27

#define FLOPPY_SECTOR_DTL_128   0
#define FLOPPY_SECTOR_DTL_256   1
#define FLOPPY_SECTOR_DTL_512   2
#define FLOPPY_SECTOR_DTL_1024  4


#define FLOPPY_NO_ERROR         0x00 
#define FLOPPY_ERR_BADCMD       0x01
#define FLOPPY_ERR_ADDR_MARK    0x02 
#define FLOPPY_ERR_PROTECTED    0x03
#define FLOPPY_ERR_SECTOR       0x04
#define FLOPPY_ERR_OVERRUN      0x08
#define FLOPPY_ERR_BOUNDARY     0x09
#define FLOPPY_ERR_MEDIA        0x0C
#define FLOPPY_ERR_CRC_ECC      0x10
#define FLOPPY_ERR_CTRL         0x20
#define FLOPPY_ERR_SEEK         0x40
#define FLOPPY_ERR_TIMEOUT      0x80
#define FLOPPY_ERR_SENSE        0xFF

void fdc_detect(void);

// uint8_t fdc_reset_controller(uint8_t drive);
// uint8_t fdc_drive_status(uint8_t drive);
// uint8_t fdc_read_sectors(const fdc_parameters __seg_ss* params);
// uint8_t fdc_write_sectors(const fdc_parameters __seg_ss* params);
// uint8_t fdc_verify_sectors(const fdc_parameters __seg_ss* params);


uint8_t fdc_send_cmd(block_t __far* blk, command_t __seg_ss* cmd);

uint8_t fdc_reset(block_t __far* blk);
uint8_t fdc_sense_media(block_t __far* blk);
// uint8_t fdc_ident_media(block_t __far* blk);
uint8_t fdc_check_media(block_t __far* blk);


uint8_t fdc_read_chs(block_t __far* blk, const command_t __seg_ss* params);
uint8_t fdc_write_chs(block_t __far* blk, const command_t __seg_ss* params);
uint8_t fdc_verify_chs(block_t __far* blk, const command_t __seg_ss* params);
uint8_t fdc_format_chs(block_t __far* blk, const command_t __seg_ss* params);