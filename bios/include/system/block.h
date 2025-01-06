#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "attrib.h"

// TODO: implement all error codes
#define BLOCK_SUCCESS       0x00
#define BLOCK_ERROR         0x01
#define BLOCK_ERR_INVALID   0x01
#define BLOCK_ERR_ADDR_MARK 0x02
#define BLOCK_ERR_MEDIA     0x06
#define BLOCK_ERR_SEEK      0x40
#define BLOCK_ERR_TIMEOUT   0x80

#define BLOCK_TYPE_RESERVED 0
#define BLOCK_TYPE_FLOPPY   1
#define BLOCK_TYPE_ATA      2 
#define BLOCK_TYPE_ATAPI    3
#define BLOCK_TYPE_SCSI     4

#define BLOCK_SUB_PRIMARY   0
#define BLOCK_SUB_SECONDARY 1
#define BLOCK_SUB_TERTIARY  2
#define BLOCK_SUB_QUATERNARY 3

// #define BLOCK_SUB_FPY_5_25 0
// #define BLOCK_SUB_FPY_3_50 1

#define BLOCK_CMD_CHS_READ   0
#define BLOCK_CMD_CHS_WRITE  1
#define BLOCK_CMD_CHS_VERIFY 2
#define BLOCK_CMD_CHS_FORMAT 3
#define BLOCK_CMD_LBA_READ   4
#define BLOCK_CMD_LBA_WRITE  5
#define BLOCK_CMD_LBA_VERIFY 6
#define BLOCK_CMD_LBA_SEEK   7
#define BLOCK_CMD_FLUSH      8
#define BLOCK_CMD_RESET      9
#define BLOCK_CMD_IDENTIFY   10
#define BLOCK_CMD_CHANGE     11

#define BLOCK_RET_CHANGE 0x01

#define BLOCK_XLATE     0x01
#define BLOCK_LBA28     0x02
#define BLOCK_LBA48     0x04
#define BLOCK_MEDIA     0x08 // Removable media present
#define BLOCK_RELOC     0x10 // Device number relocated
#define BLOCK_EMULA     0x20 // Emulating device
#define BLOCK_RECAL     0x40 // Recalibrate
#define BLOCK_VALID     0x80

#define CMD_EXT 0x80

typedef struct geometry geometry_t;
typedef struct block    block_t;
typedef struct command  command_t;

typedef struct fixed_disk_parameter_table fdpt_t;
typedef struct diskette_parameter_table   dkpt_t;

struct __packed geometry
{
    uint16_t cylinders;
    uint16_t heads;
    uint16_t sectors;
};

struct __packed command
{
    uint8_t    cmd; // Command
    uint8_t    flg; // Flags
    union {
    uint64_t   lba; // LBA address
    geometry_t chs;
    };
    uint32_t   cnt; // sector count
    uint32_t   buf; // Linear buffer address
};

typedef uint8_t (*send_cmd_t)(block_t __far* blk, command_t __seg_ss* cmd);

struct __packed fixed_disk_parameter_table
{
    uint16_t logical_cylinders;     // Limit: 1023
    uint8_t  logical_heads;         // Limit: 255
    uint8_t  signature;             // 0xA0
    uint8_t  physical_sectors;
    uint16_t precompensation;
    uint8_t  __reserved;
    uint8_t  drive_control;
    uint16_t physical_cylinders;    // Limit: 65535
    uint8_t  physical_heads;
    uint16_t landing_zone;
    uint8_t  logical_sectors;       // Limit: 63
    uint8_t  checksum;
};

struct __packed diskette_parameter_table
{
    uint8_t specify_0;
    uint8_t specify_1;
    uint8_t motor_shutoff;
    uint8_t bytes_per_sector;
    uint8_t sector_count;
    uint8_t gap_length;
    uint8_t data_length;
    uint8_t format_gap_length;
    uint8_t fill_byte;
    uint8_t head_settle_time;
    uint8_t motor_start_delay;
    uint8_t track_count;
    uint8_t data_rate;
    uint8_t drive_type;
};

struct __packed block
{
    uint8_t     id;     // Actual device number
    uint8_t     alt;    // Alternate device number
    uint8_t     type;   // Device type
    uint8_t     sub;    // Subtype (e.g. Primary IDE device, SCSI LUN)
    uint8_t     flags;  // Flags
    uint64_t    lba;    // LBA address
    uint32_t    off;    // LBA offset
    geometry_t  geom;   // Drive geometry
    uint16_t    size;   // Size of the block in bytes
    uint16_t    io;     // I/O port
    uint8_t     desc[32]; // Description

    union {
    fdpt_t      fdpt;   // Fixed Disk Parameter Table
    dkpt_t      dkpt;   // Diskette Parameter Table
    };

    send_cmd_t  send_cmd;
};

// struct __packed fixed_disk_parameter_table_extension
// {
//     uint16_t iobase;
//     uint16_t control;
//     uint8_t  
// }

// TODO: consistent return values (uint8_t)

uint8_t block_create(uint8_t type, uint8_t sub, uint8_t flags, uint64_t lba,
    geometry_t geom, uint16_t size, uint16_t io, uint8_t __far* desc);

bool block_find(uint8_t id, block_t __far*__far* blk);
bool block_has(uint8_t id);
bool block_remove(uint8_t id);
bool block_insert(block_t __far* blk);

uint8_t block_generate_fdpt(block_t __far* blk);
uint8_t block_generate_dkpt(block_t __far* blk);

bool block_translate_none(fdpt_t __far* fdpt, geometry_t geom);
bool block_translate_chs(fdpt_t __far* fdpt, geometry_t geom);
bool block_translate_lba(fdpt_t __far* fdpt, geometry_t geom, uint64_t lba);
void block_fdpt_create_checksum(fdpt_t __far* fdpt);
bool block_fdpt_verify_checksum(fdpt_t __far* fdpt);

void block_untranslate_lchs(const block_t __far* blk,
    const geometry_t __far* lchs, uint32_t __far* lba);
void block_untranslate_pchs(const block_t __far* blk, 
    const uint32_t __far* lba, geometry_t __far* pchs);

uint8_t block_count(void);
uint8_t block_floppy_count(void);
uint8_t block_disk_count(void);

uint8_t block_emulate_drive_fixed(block_t __far* blk, uint8_t type, uint32_t offset);
uint8_t block_emulate_drive_floppy(block_t __far* blk, uint8_t type, uint32_t offset);

void block_list_all(void);
void block_list_all_fdpt(void);