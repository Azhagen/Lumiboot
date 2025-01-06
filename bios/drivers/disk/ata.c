#include "drivers/disk/ata.h"

#include "io.h"
#include "debug.h"
#include "utility.h"
#include "system/block.h"
#include "system/wait.h"

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

#define ATA_CMD_IDENTIFY    0xEC
#define ATA_CMD_READ_PIO    0x20
#define ATA_CMD_WRITE_PIO   0x30
#define ATA_CMD_VERIFY_RD   0x40
#define ATA_CMD_FORMAT      0x50

#define ATA_SUCCESS         0x00
#define ATA_ERR_UNAVAIL     0x01

#define ATA_IDENTIFY_LBA    (1 << 9)
#define ATA_IDENTIFY_DMA    (1 << 8)

static uint8_t __far* ata_string(uint8_t __far* src, size_t size)
{
    for (size_t i = 0; i < size; i += 2)
    {
        uint8_t tmp = src[i];
        src[i] = src[i + 1];
        src[i + 1] = tmp;
    }

    if (size > 0)
        src[size] = 0;

    return src;
}

static void ata_write(uint16_t io, uint8_t reg, uint8_t value)
{
    io_write(io + reg, value);
}

static bool ata_wait_busy(uint16_t io)
{
    uint8_t status = io_read(io + ATA_STATUS);

    while ((status & 0x80) == 0x80)
        status = io_read(io + ATA_STATUS);

    return true;
}

static bool ata_wait_busy_delay(uint16_t io)
{
    for (size_t i = 0; i < 14; ++i)
        io_read(io + ATA_STATUS);
    return ata_wait_busy(io);
}

static bool ata_poll(uint16_t io)
{
    uint8_t status = io_read(io + ATA_STATUS);

    while ((status & 0x80) != 0x00)
        status = io_read(io + ATA_STATUS);

    while ((status & 0x08) != 0x08)
        status = io_read(io + ATA_STATUS);

    return (status & 0x01) == 0;
}

static uint8_t ata_identify(uint16_t device, bool primary)
{
    uint8_t drive = primary ? 0xA0 : 0xB0;

    ata_write(device, ATA_DRIVE, drive);
    ata_write(device, ATA_COUNT, 0);
    ata_write(device, ATA_LBA_LO, 0);
    ata_write(device, ATA_LBA_MI, 0);
    ata_write(device, ATA_LBA_HI, 0);
    ata_write(device, ATA_COMMAND, ATA_CMD_IDENTIFY);

    uint8_t status = io_read(device + ATA_STATUS);
    if (status == 0x00 || status == 0xFF)
        return BLOCK_ERROR;

    if (io_read(device + ATA_LBA_HI) != 0 ||
        io_read(device + ATA_LBA_MI) != 0 ||
        io_read(device + ATA_LBA_LO) != 0)
        return BLOCK_ERROR;

    if (!ata_poll(device))
        return BLOCK_ERROR;

    uint16_t __far* buffer = (uint16_t __far*)0x00007C00L;
    for (size_t i = 0; i < 256; ++i)
        buffer[i] = io_read16(device + ATA_DATA);

    ata_identity_t __far* identity = (ata_identity_t __far*)buffer;
    uint8_t __far* str = ata_string(identity->model, 40);
    debug_out("[BIOS] Model: %s\n\r", str);

    uint8_t sub = primary ? BLOCK_SUB_PRIMARY : BLOCK_SUB_SECONDARY;
    geometry_t geom = { identity->cylinders, identity->heads, identity->sectors };
    uint32_t lba = identity->lba_capacity;
    uint8_t flag = identity->capabilities & ATA_IDENTIFY_LBA ? BLOCK_LBA28 : 0;

    debug_out("[BIOS] LBA capacity: %lu\n\r", lba);

    if (block_create(BLOCK_TYPE_ATA, sub, flag, lba, geom, 512, device, str) != BLOCK_SUCCESS)
        return BLOCK_ERROR;

    debug_out("[BIOS] Created ATA device\n\r");

    return BLOCK_SUCCESS;
}

void ata_detect(void)
{
    ata_identify(0x1F0, true);
    ata_identify(0x1F0, false);
    ata_identify(0x170, true);
    ata_identify(0x170, false);
}

void ata_translate(block_t __far* blk, command_t __seg_ss* cmd)
{
    uint32_t lba = 0;
    block_untranslate_lchs(blk, &cmd->chs, &lba);
    block_untranslate_pchs(blk, &lba, &cmd->chs);
}

uint8_t ata_send_cmd(block_t __far* blk, command_t __seg_ss* cmd)
{
    // If the drive does not support LBA, translate to CHS
    if ((cmd->flg & CMD_EXT) && !(blk->flags & (BLOCK_LBA28 | BLOCK_LBA48)))
    {
        geometry_t chs = {};
        uint32_t   lba = (uint32_t)cmd->lba;
        block_untranslate_pchs(blk, &lba, &chs);

        debug_out("[BIOS] ATA: translated LBA %lu to CHS %d:%d:%d\n\r",
            (uint32_t)cmd->lba, chs.cylinders, chs.heads, chs.sectors);

        cmd->chs = chs;
        cmd->cmd = BLOCK_CMD_CHS_READ;
    }

    switch (cmd->cmd)
    {
        case BLOCK_CMD_RESET:
            return ata_reset(blk);
        case BLOCK_CMD_CHS_READ:
            if (blk->flags & BLOCK_XLATE) ata_translate(blk, cmd);
            return ata_read_chs(blk, cmd->chs.cylinders, cmd->chs.heads, cmd->chs.sectors, (uint8_t)cmd->cnt, cmd->buf);
        case BLOCK_CMD_CHS_WRITE:
            if (blk->flags & BLOCK_XLATE) ata_translate(blk, cmd);
            return ata_write_chs(blk, cmd->chs.cylinders, cmd->chs.heads, cmd->chs.sectors, (uint8_t)cmd->cnt, cmd->buf);
        case BLOCK_CMD_CHS_VERIFY:
            if (blk->flags & BLOCK_XLATE) ata_translate(blk, cmd);
            return ata_verify_chs(blk, cmd->chs.cylinders, cmd->chs.heads, cmd->chs.sectors, (uint8_t)cmd->cnt);
        case BLOCK_CMD_CHS_FORMAT:
            if (blk->flags & BLOCK_XLATE) ata_translate(blk, cmd);
            return ata_format_chs(blk, cmd->chs.cylinders, cmd->chs.heads, cmd->chs.sectors, (uint8_t)cmd->cnt, cmd->buf);
        case BLOCK_CMD_LBA_READ:
            return ata_read_lba(blk, (uint32_t)cmd->lba, cmd->cnt, cmd->buf);
        default:
            break;
    }

    return BLOCK_ERROR;
}

uint8_t ata_reset(block_t __far* blk)
{
    // TODO: Implement ATA reset
    return BLOCK_SUCCESS;
}

uint8_t ata_read_chs(block_t __far* blk, uint16_t cylinder, uint16_t head,
    uint16_t sector, uint8_t count, uint32_t buffer)
{
    // debug_out("[BIOS] Reading %d sectors from CHS %d:%d:%d\n\r",
    //     count, cylinder, head, sector);

    uint8_t drive = blk->sub == BLOCK_SUB_PRIMARY ? 0xA0 : 0xB0;
    ata_write(blk->io, ATA_DRIVE, (uint8_t)(drive | (head & 0x0F)));

    ata_write(blk->io, ATA_FEATURES, 0x00);
    ata_write(blk->io, ATA_COUNT, count);
    ata_write(blk->io, ATA_LBA_LO, (uint8_t)sector);
    ata_write(blk->io, ATA_LBA_MI, (uint8_t)(cylinder & 0xFF));
    ata_write(blk->io, ATA_LBA_HI, (uint8_t)(cylinder >> 8));
    ata_write(blk->io, ATA_COMMAND, ATA_CMD_READ_PIO);

    uint16_t __far* buf = linear_to_fp(buffer);

    for (size_t i = 0; i < count; ++i)
    {
        if (!ata_poll(blk->io))
            return BLOCK_ERROR;

        for (size_t j = 0; j < 256; ++j)
            buf[j] = io_read16(blk->io + ATA_DATA);

        buf += 256;
    }

    return BLOCK_SUCCESS;
}

uint8_t ata_write_chs(block_t __far* blk, uint16_t cylinder, uint16_t head,
    uint16_t sector, uint8_t count, uint32_t buffer)
{
    // debug_out("[BIOS] Writing %d sectors to CHS %d:%d:%d\n\r",
    //     count, cylinder, head, sector);

    uint8_t drive = blk->sub == BLOCK_SUB_PRIMARY ? 0xA0 : 0xB0;
    ata_write(blk->io, ATA_DRIVE, (uint8_t)(drive | (head & 0x0F)));

    ata_write(blk->io, ATA_FEATURES, 0x00);
    ata_write(blk->io, ATA_COUNT, count);
    ata_write(blk->io, ATA_LBA_LO, (uint8_t)sector);
    ata_write(blk->io, ATA_LBA_MI, (uint8_t)(cylinder & 0xFF));
    ata_write(blk->io, ATA_LBA_HI, (uint8_t)(cylinder >> 8));
    ata_write(blk->io, ATA_COMMAND, ATA_CMD_WRITE_PIO);

    uint16_t __far* buf = linear_to_fp(buffer);

    for (size_t i = 0; i < count; ++i)
    {
        if (!ata_poll(blk->io))
            return BLOCK_ERROR;

        for (size_t j = 0; j < 256; ++j)
            io_write16(blk->io + ATA_DATA, buf[j]);

        buf += 256;
    }

    ata_write(blk->io, ATA_COMMAND, 0xE7);
    while (io_read(blk->io + ATA_STATUS) & 0x80);

    return BLOCK_SUCCESS;
}

uint8_t ata_verify_chs(block_t __far* blk, uint16_t cylinder, uint16_t head,
    uint16_t sector, uint8_t count)
{
    uint8_t drive = blk->sub == BLOCK_SUB_PRIMARY ? 0xA0 : 0xB0;
    ata_write(blk->io, ATA_DRIVE, (uint8_t)(drive | (head & 0x0F)));

    ata_write(blk->io, ATA_FEATURES, 0x00);
    ata_write(blk->io, ATA_COUNT, count);
    ata_write(blk->io, ATA_LBA_LO, (uint8_t)sector);
    ata_write(blk->io, ATA_LBA_MI, (uint8_t)(cylinder & 0xFF));
    ata_write(blk->io, ATA_LBA_HI, (uint8_t)(cylinder >> 8));
    ata_write(blk->io, ATA_COMMAND, ATA_CMD_VERIFY_RD);

    if (!ata_poll(blk->io))
        return BLOCK_ERROR;

    return BLOCK_SUCCESS;
}

uint8_t ata_format_chs(block_t __far* blk, uint16_t cylinder, uint16_t head,
    uint16_t sector, uint8_t count, uint32_t buffer)
{
    uint8_t drive = blk->sub == BLOCK_SUB_PRIMARY ? 0xA0 : 0xB0;
    ata_write(blk->io, ATA_DRIVE, (uint8_t)(drive | (head & 0x0F)));

    ata_write(blk->io, ATA_FEATURES, 0x00);
    ata_write(blk->io, ATA_COUNT, count);
    ata_write(blk->io, ATA_LBA_LO, (uint8_t)sector);
    ata_write(blk->io, ATA_LBA_MI, (uint8_t)(cylinder & 0xFF));
    ata_write(blk->io, ATA_LBA_HI, (uint8_t)(cylinder >> 8));
    ata_write(blk->io, ATA_COMMAND, ATA_CMD_FORMAT);

    if (!ata_poll(blk->io))
        return BLOCK_ERROR;

    uint16_t __far* buf = linear_to_fp(buffer);

    for (size_t i = 0; i < 256; ++i)
    {
        io_write16(blk->io + ATA_DATA, buf[i]);

        if (!ata_poll(blk->io))
            return BLOCK_ERROR;
    }

    return BLOCK_SUCCESS;
}

uint8_t ata_read_lba(block_t __far* blk, uint32_t lba, uint32_t count, uint32_t buffer)
{
    // debug_out("[BIOS] Reading %lu sectors from LBA %lu\n\r", count, lba);

    uint8_t drive = blk->sub == BLOCK_SUB_PRIMARY ? 0xE0 : 0xF0;
    ata_write(blk->io, ATA_DRIVE, (uint8_t)(drive | ((lba >> 24) & 0x0F)));

    ata_write(blk->io, ATA_FEATURES, 0x00);
    ata_write(blk->io, ATA_COUNT, (uint8_t)count);
    ata_write(blk->io, ATA_LBA_LO, (uint8_t)(lba & 0xFF));
    ata_write(blk->io, ATA_LBA_MI, (uint8_t)((lba >> 8) & 0xFF));
    ata_write(blk->io, ATA_LBA_HI, (uint8_t)((lba >> 16) & 0xFF));
    ata_write(blk->io, ATA_COMMAND, ATA_CMD_READ_PIO);

    uint16_t __far* buf = linear_to_fp(buffer);

    for (size_t i = 0; i < count; ++i)
    {
        if (!ata_poll(blk->io))
            return BLOCK_ERROR;

        for (size_t j = 0; j < 256; ++j)
            buf[j] = io_read16(blk->io + ATA_DATA);

        buf += 256;
    }

    return BLOCK_SUCCESS;
}

uint8_t ata_write_lba(block_t __far* blk, uint32_t lba, uint32_t count, uint32_t buffer)
{
    // debug_out("[BIOS] Writing %lu sectors to LBA %lu\n\r", count, lba);

    uint8_t drive = blk->sub == BLOCK_SUB_PRIMARY ? 0xE0 : 0xF0;
    ata_write(blk->io, ATA_DRIVE, (uint8_t)(drive | ((lba >> 24) & 0x0F)));

    ata_write(blk->io, ATA_FEATURES, 0x00);
    ata_write(blk->io, ATA_COUNT, (uint8_t)count);
    ata_write(blk->io, ATA_LBA_LO, (uint8_t)(lba & 0xFF));
    ata_write(blk->io, ATA_LBA_MI, (uint8_t)((lba >> 8) & 0xFF));
    ata_write(blk->io, ATA_LBA_HI, (uint8_t)((lba >> 16) & 0xFF));
    ata_write(blk->io, ATA_COMMAND, ATA_CMD_WRITE_PIO);

    uint16_t __far* buf = linear_to_fp(buffer);

    for (size_t i = 0; i < count; ++i)
    {
        if (!ata_poll(blk->io))
            return BLOCK_ERROR;

        for (size_t j = 0; j < 256; ++j)
            io_write16(blk->io + ATA_DATA, buf[j]);

        buf += 256;
    }

    ata_write(blk->io, ATA_COMMAND, 0xE7);
    while (io_read(blk->io + ATA_STATUS) & 0x80);

    return BLOCK_SUCCESS;
}

uint8_t ata_verify_lba(block_t __far* blk, uint32_t lba, uint32_t count)
{
    uint8_t drive = blk->sub == BLOCK_SUB_PRIMARY ? 0xE0 : 0xF0;
    ata_write(blk->io, ATA_DRIVE, (uint8_t)(drive | ((lba >> 24) & 0x0F)));

    ata_write(blk->io, ATA_FEATURES, 0x00);
    ata_write(blk->io, ATA_COUNT, (uint8_t)count);
    ata_write(blk->io, ATA_LBA_LO, (uint8_t)(lba & 0xFF));
    ata_write(blk->io, ATA_LBA_MI, (uint8_t)((lba >> 8) & 0xFF));
    ata_write(blk->io, ATA_LBA_HI, (uint8_t)((lba >> 16) & 0xFF));
    ata_write(blk->io, ATA_COMMAND, ATA_CMD_VERIFY_RD);

    if (!ata_poll(blk->io))
        return BLOCK_ERROR;

    return BLOCK_SUCCESS;
}

uint8_t ata_seek_lba(block_t __far* blk, uint32_t lba)
{
    uint8_t drive = blk->sub == BLOCK_SUB_PRIMARY ? 0xE0 : 0xF0;
    ata_write(blk->io, ATA_DRIVE, (uint8_t)(drive | ((lba >> 24) & 0x0F)));

    ata_write(blk->io, ATA_FEATURES, 0x00);
    ata_write(blk->io, ATA_COUNT, 0);
    ata_write(blk->io, ATA_LBA_LO, (uint8_t)(lba & 0xFF));
    ata_write(blk->io, ATA_LBA_MI, (uint8_t)((lba >> 8) & 0xFF));
    ata_write(blk->io, ATA_LBA_HI, (uint8_t)((lba >> 16) & 0xFF));
    ata_write(blk->io, ATA_COMMAND, 0x70);

    if (!ata_poll(blk->io))
        return BLOCK_ERROR;

    return BLOCK_SUCCESS;
}