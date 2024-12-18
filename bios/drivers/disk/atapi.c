#include "drivers/disk/atapi.h"

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "attrib.h"
#include "io.h"
#include "debug.h"
#include "utility.h"
#include "system/block.h"
#include "system/data.h"

#include "bios.h"
#include "string.h"
#include "print.h"

#define ATA_DATA        0x00
#define ATA_ERROR       0x01
#define ATA_FEATURES    0x01
#define ATA_COUNT       0x02
#define ATA_LBA_LO      0x03
#define ATA_LBA_MI      0x04
#define ATA_LBA_HI      0x05
#define ATA_DRIVE       0x06
#define ATA_STATUS      0x07
#define ATA_COMMAND     0x07

#define ATAPI_IDENTIFY 0xA1
#define ATAPI_DIAGNOSE 0x90

// Status flags
#define ATAPI_STATUS_BSY 0x80
#define ATAPI_STATUS_RDY 0x40
#define ATAPI_STATUS_DRQ 0x08
#define ATAPI_STATUS_ERR 0x01

// TODO: implement the struct fully
struct atapi_identity
{
    uint16_t general;
    uint16_t __reserved0[9];
    uint8_t  serial[20];
    uint16_t __reserved1[3];
    uint8_t  firmware[8];
    uint8_t  model[40];
    uint16_t __reserved2[2];
    uint16_t capabilities;
    uint16_t __reserved3;
};

typedef struct atapi_identity atapi_identity_t;

static char __far* atapi_string(char __far* src, size_t size)
{
    for (size_t i = 0; i < size; i += 2)
    {
        uint8_t tmp = src[i + 0];
        src[i + 0] = src[i + 1];
        src[i + 1] = tmp;
    }

    if (size > 0)
        src[size] = 0;

    return src;
}

// Wait for the drive to be ready
static bool atapi_wait_ready(uint16_t device)
{
    while (true)
    {
        uint8_t status = io_read(device + ATA_STATUS);
        if ((status & ATAPI_STATUS_ERR) == ATAPI_STATUS_ERR)
            return false;
        if (!(status & ATAPI_STATUS_BSY) && (status & ATAPI_STATUS_DRQ))
            break;
    }

    return true;
}

uint8_t atapi_diagnose(uint16_t device, bool primary)
{
    uint8_t drive = primary ? 0xA0 : 0xB0;
    debug_out("drive: %02X\n\r", drive);

    // Select the drive
    io_write(device + ATA_DRIVE, drive);

    if ((io_read(device + ATA_STATUS) & 0x01) == 0x01)
        return BLOCK_ERROR;

    // Send the command
    io_write(device + ATA_COMMAND, ATAPI_DIAGNOSE);

    // Wait for the drive to be ready
    uint8_t status = io_read(device + ATA_STATUS);
    if (status == 0x00 || status == 0xFF)
        return BLOCK_ERROR;

    while ((status & 0x80) != 0x00)
        status = io_read(device + ATA_STATUS);

    if (io_read(device + ATA_ERROR) != 0x01)
        return BLOCK_ERROR;

    return BLOCK_SUCCESS;
}

uint8_t atapi_identify(uint16_t device, bool primary, void __far* buffer)
{
    uint8_t drive = primary ? 0xA0 : 0xB0;

    // Select the drive
    io_write(device + ATA_DRIVE, drive);
    
    if ((io_read(device + ATA_STATUS) & 0x01) == 0x01)
        return BLOCK_ERROR;
    
    if (io_read(device + ATA_LBA_HI) != 0xEB ||
        io_read(device + ATA_LBA_MI) != 0x14 ||
        io_read(device + ATA_LBA_LO) != 0x01)
        return BLOCK_ERROR;

    // Send the command
    io_write(device + ATA_FEATURES, 0x00);
    io_write(device + ATA_COUNT, 0);
    io_write(device + ATA_LBA_LO, 0);
    io_write(device + ATA_LBA_MI, 0);
    io_write(device + ATA_LBA_HI, 0);
    io_write(device + ATA_COMMAND, ATAPI_IDENTIFY);

    if (!atapi_wait_ready(device))
        return BLOCK_ERROR;

    uint16_t __far* buf = (uint16_t __far*)buffer;

    for (size_t i = 0; i < 256; ++i)
        buf[i] = io_read16(device + ATA_DATA);

    return BLOCK_SUCCESS;
}

bool atapi_send_packet(uint16_t device, uint8_t __far* packet, uint16_t size)
{
    // Send the command
    io_write(device + ATA_FEATURES, 0x00);
    io_write(device + ATA_COUNT,    0x00);
    io_write(device + ATA_LBA_LO,   0x00);
    io_write(device + ATA_LBA_MI,   0x08);
    io_write(device + ATA_LBA_HI,   0x00);
    io_write(device + ATA_COMMAND,  0xA0);

    // Wait for the drive to be ready
    if (!atapi_wait_ready(device))
        return false;

    // Send the packet
    for (size_t i = 0; i < size / 2; ++i)
    {
        uint16_t value = as_uint16(packet[2 * i + 1], packet[2 * i]);
        debug_out("[BIOS] Sending packet: %04X\n\r", value);
        io_write16(device + ATA_DATA, value);
    }

    return true;
}

uint8_t atapi_read_capacity(uint16_t device, bool primary,
    uint32_t __seg_ss* capacity, uint32_t __seg_ss* block_size)
{
    uint8_t  packet[12]  = {0};
    uint16_t response[4] = {0};

    packet[0] = 0x25; // READ CAPACITY command

    uint8_t drive = primary ? 0xA0 : 0xB0;

    // Select the drive
    io_write(device + ATA_DRIVE, drive);

    if ((io_read(device + ATA_STATUS) & 0x01) == 0x01)
        return BLOCK_ERROR;

    if (!atapi_send_packet(device, packet, 12))
        return BLOCK_ERROR;

    // Wait for the drive to be ready
    if (!atapi_wait_ready(device))
        return BLOCK_ERROR;

    // Read the data
    for (size_t i = 0; i < 4; ++i)
        response[i] = io_read16(device + ATA_DATA);

    *capacity   = to_le32(as_uint32(response[1], response[0]));
    *block_size = to_le32(as_uint32(response[3], response[2]));

    return BLOCK_SUCCESS;
}

void atapi_add_device(uint16_t device, bool primary, void __far* buffer)
{
    uint8_t  sub  = primary ? BLOCK_SUB_PRIMARY : BLOCK_SUB_SECONDARY;
    uint8_t  flag = BLOCK_LBA28 | BLOCK_MEDIA;
    uint32_t lba  = 0;
    uint32_t size = 0;
    geometry_t geom = {0, 0, 0};

    if (atapi_read_capacity(device, primary, &lba, &size) != BLOCK_SUCCESS)
        return;

    debug_out("[BIOS] ATAPI Device: %lu sectors, %lu bytes/sector\n\r", lba, size);

    atapi_identity_t __far* identity = (atapi_identity_t __far*)buffer;
    char __far* str = atapi_string(identity->model, 40);

    if (!block_create(BLOCK_TYPE_ATAPI, sub, flag, lba, geom, 2048, device, str))
        return;
}

void atapi_detect(void)
{
    void __far* buffer = (void __far*)0x00007C00L;

    atapi_diagnose(0x1F0, true);
    if (atapi_identify(0x1F0, true, buffer) == BLOCK_SUCCESS)
        atapi_add_device(0x1F0, true, buffer);

    atapi_diagnose(0x1F0, false);
    if (atapi_identify(0x1F0, false, buffer) == BLOCK_SUCCESS)
        atapi_add_device(0x1F0, false, buffer);

    // atapi_diagnose(0x170, true);
    // atapi_identify(0x170, true);
    // atapi_diagnose(0x170, false);
    // atapi_identify(0x170, false); 
}

extern void boot_start(uint8_t drive);

struct __packed eltorito_validation_entry
{
    uint8_t  header_id;
    uint8_t  platform_id;
    uint16_t __reserved;
    uint8_t  id_string[24];
    uint16_t checksum;
    uint16_t key;
};

typedef struct eltorito_validation_entry eltorito_validation_entry_t;

static_assert(sizeof(eltorito_validation_entry_t) == 32, "Invalid size");

struct __packed eltorito_boot_entry
{
    uint8_t  indicator;
    uint8_t  media_type;
    uint16_t load_segment;
    uint8_t  system_type;
    uint8_t  __reserved0;
    uint16_t sector_count;
    uint32_t load_lba;
    uint8_t  __reserved1[20];
};

typedef struct eltorito_boot_entry eltorito_boot_entry_t;

static_assert(sizeof(eltorito_boot_entry_t) == 32, "Invalid size");

void __far atapi_boot(void)
{
    while (true);

    // uint16_t device = get_ebda()->boot_device;
    // if ((device & 0xFF) != BLOCK_TYPE_ATAPI)
    //     return;

    // uint8_t drive = (uint8_t)(device >> 8);

    // block_t __far* blk = 0;
    // if (!block_find(drive, &blk))
    //     return;

    // command_t cmd = {0};
    // cmd.command = BLOCK_CMD_READ;
    // cmd.lba     = 0x11;
    // cmd.count   = 1;
    // cmd.buffer  = (void __far*)0x7C00L;

    // if (blk->send_cmd(blk, &cmd) != BLOCK_SUCCESS)
    //     return;

    // // io_write(0x80, 0xAA);

    // eltorito_validation_entry_t __far* validation = (eltorito_validation_entry_t __far*)0x00007C00L;

    // // if (validation->header_id != 0x01)
    // //     return;
    // // if (validation->platform_id != 0x00)
    // //     return;


    // eltorito_boot_entry_t __far* boot = (eltorito_boot_entry_t __far*)(validation + 1);

    // io_write(0x80, boot->indicator);

    // if (boot->indicator != 0x88)
    //     return;

    // uint32_t lba = boot->load_lba;
    // uint16_t count = boot->sector_count;

    // cmd.command = BLOCK_CMD_READ;
    // cmd.lba     = lba;
    // cmd.count   = count;
    // cmd.buffer  = (void __far*)0x7C00L;

    // blk->send_cmd(blk, &cmd);

    // boot_start(drive);
}

static uint8_t atapi_read(block_t __far* blk, uint32_t lba,
    uint32_t count, void __far* buffer)
{
    uint8_t __far* buf = buffer;

    for (uint32_t i = 0; i < count; ++i)
    {
        uint32_t read_lba = lba + i;
        uint8_t  off      = 0;

        if (blk->flags & BLOCK_EMULA)
        {
            read_lba = blk->off + (lba + i) / 4; // Map to real LBA
            off      = (lba + i) % 4;            // Offset within real sector
            buf      = get_ebda()->buffer;       // Use intermediate buffer for emulation
        }
        else
        {
            buf = (uint8_t __far*)buffer + i * 2048; // Directly write to caller's buffer
        }

        if (atapi_read_lba(blk, read_lba, 1, buf) != BLOCK_SUCCESS)
            return BLOCK_ERROR;

        if (blk->flags & BLOCK_EMULA)
            fmemcpy_16(buffer + i * 512, buf + off * 512, 256);
    }

    return BLOCK_SUCCESS;
}

geometry_t atapi_translate(block_t __far* blk, geometry_t chs)
{
    if ((blk->flags & BLOCK_XLATE) != 0)
        return chs;

    uint32_t lba = 0;
    block_untranslate_lchs(blk, &chs, &lba);
    block_untranslate_pchs(blk, &lba, &chs);
    return chs;

    // uint32_t lba = block_untranslate_lba(blk, chs.cylinders, chs.heads, chs.sectors);
    // return block_untranslate_pchs(blk, lba);
}



uint8_t atapi_send_cmd(block_t __far* blk, command_t __seg_ss* cmd)
{
    if (blk->flags & BLOCK_EMULA)
    {


        switch (cmd->cmd)
        {
            case BLOCK_CMD_CHS_READ:
                return atapi_read(blk, (uint32_t)cmd->lba, cmd->cnt, cmd->buf);

            default: return BLOCK_ERROR;
        }
    }



    return BLOCK_ERROR;
}



uint8_t atapi_read_lba(block_t __far* blk, uint32_t lba,
    uint32_t count, void __far* buffer)
{
    uint16_t sector_size = 2048;
    uint8_t packet[12] = {0};

    packet[0] = 0xA8;
    packet[2] = (uint8_t)((lba >> 24) & 0xFF);
    packet[3] = (uint8_t)((lba >> 16) & 0xFF);
    packet[4] = (uint8_t)((lba >> 8) & 0xFF);
    packet[5] = (uint8_t)((lba >> 0) & 0xFF);
    packet[6] = (uint8_t)((count >> 24) & 0xFF);
    packet[7] = (uint8_t)((count >> 16) & 0xFF);
    packet[8] = (uint8_t)((count >> 8) & 0xFF);
    packet[9] = (uint8_t)((count >> 0) & 0xFF);

    uint8_t drive = blk->sub == BLOCK_SUB_PRIMARY ? 0xA0 : 0xB0;

    // Select the drive
    io_write(blk->io + ATA_DRIVE, drive);

    if ((io_read(blk->io + ATA_STATUS) & 0x01) == 0x01)
        return BLOCK_ERROR;

    // Send the command
    io_write(blk->io + ATA_ERROR,  0x00);
    io_write(blk->io + ATA_LBA_MI, (uint8_t)(sector_size & 0xFF));
    io_write(blk->io + ATA_LBA_HI, (uint8_t)(sector_size >> 8));
    io_write(blk->io + ATA_COMMAND, 0xA0);

    if (!atapi_wait_ready(blk->io))
        return BLOCK_ERROR;

    // Send the packet
    for (size_t i = 0; i < 12 / 2; ++i)
    {
        uint16_t value = as_uint16(packet[2 * i + 1], packet[2 * i]);
        // debug_out("[BIOS] Sending packet: %04X\n\r", value);
        io_write16(blk->io + ATA_DATA, value);
    }

    // Read the data
    uint16_t __far* buf = (uint16_t __far*)buffer;

    for (size_t i = 0; i < count; ++i)
    {
        // Wait for the drive to be ready
        if (!atapi_wait_ready(blk->io))
            return BLOCK_ERROR;

        // Check the transfer size for this sector
        uint16_t transfer_size = as_uint16(
            io_read(blk->io + ATA_LBA_HI),
            io_read(blk->io + ATA_LBA_MI)
        );

        for (size_t j = 0; j < transfer_size / 2; ++j)
            buf[j] = io_read16(blk->io + ATA_DATA);

        buf += transfer_size / 2;

        if (io_read(blk->io + ATA_STATUS) & 0x01)
            return BLOCK_ERROR;
    }

    return BLOCK_SUCCESS;
}

uint8_t atapi_write_lba(block_t __far* blk, uint64_t lba,
    uint8_t count, void __far* buffer)
{
    return BLOCK_ERROR;
}

uint8_t atapi_verify_lba(block_t __far* blk, uint64_t lba,
    uint8_t count)
{
    return BLOCK_ERROR;
}

uint8_t atapi_format_lba(block_t __far* blk, uint64_t lba,
    uint8_t count, void __far* buffer)
{
    return BLOCK_ERROR;
}
