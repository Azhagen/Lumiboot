#include "system/block.h"
#include "system/data.h"

#include "drivers/disk/fdc.h"
#include "drivers/disk/ata.h"
#include "drivers/disk/atapi.h"

#include "debug.h"
#include "string.h"

// https://wiki.preterhuman.net/BIOS_Types,_CHS_Translation,_LBA_and_Other_Good_Stuff
// https://github.com/coreboot/seabios/blob/master/src/block.c

static bool block_valid(block_t __far* blk)
{
    return blk->type != BLOCK_TYPE_RESERVED;
}

bool block_has(uint8_t id)
{
    block_t __far* table = get_ebda()->block_table;

    for (uint8_t i = 0; i < 32; ++i)
    {
        if (table[i].id == id)
            return true;
    }

    return false;
}

bool block_find(uint8_t id, block_t __far*__far* blk)
{
    block_t __far* table = get_ebda()->block_table;

    for (uint8_t i = 0; i < 32; ++i)
    {
        if (!block_valid(&table[i]))
            continue;

        if (table[i].id != id)
            continue;

        *blk = &table[i];
        return true;
    }

    return false;
}

// insert a block into the block table and return the index
bool block_insert(block_t __far* blk)
{
    block_t __far* table = get_ebda()->block_table;

    for (uint8_t i = 0; i < 32; ++i)
    {
        if (!block_valid(&table[i]))
        {
            table[i] = *blk;
            return true;
        }
    }

    return false;
}

bool block_remove(uint8_t id)
{
    block_t __far* table = get_ebda()->block_table;

    for (uint8_t i = 0; i < 32; ++i)
    {
        if (table[i].id == id)
        {
            table[i].flags &= (uint8_t)~BLOCK_VALID;
            return true;
        }
    }

    return false;
}

static uint8_t block_find_next_id(uint8_t type)
{
    block_t __far* table = get_ebda()->block_table;

    uint8_t cur = 0x00;
    uint8_t found = 0;

    if (type != BLOCK_TYPE_FLOPPY)
        cur = 0x80;

    for (uint8_t i = 0; i < 32; ++i)
    {
        if (!block_valid(&table[i]))
            continue;

        if (type != BLOCK_TYPE_FLOPPY)
        {
            if (table[i].id < 0x80)
                continue;
        }
        else
        {
            if (table[i].id >= 0x80)
                continue;
        }

        if (table[i].id > cur)
            cur = table[i].id;
        
        found++;
    }

    // debug_out("found %u devices\n\r", found);

    if (found == 0)
        return cur;
    else
        return ++cur;
}

bool block_copy_desc(block_t __far* blk, uint8_t __far* desc)
{
    if (desc == NULL)
        return false;

    for (size_t i = 0; i < 32; ++i)
    {
        if (desc[i] == '\0')
            break;

        blk->desc[i] = desc[i];
    }

    blk->desc[31] = '\0';

    return true;
}

static send_cmd_t block_set_command(uint8_t type)
{
    switch (type)
    {
        case BLOCK_TYPE_FLOPPY: return fdc_send_cmd;
        case BLOCK_TYPE_ATA:    return ata_send_cmd;
        case BLOCK_TYPE_ATAPI:  return atapi_send_cmd;
        default: return NULL;
    }
}

uint8_t block_create(uint8_t type, uint8_t sub, uint8_t flags, uint64_t lba,
    geometry_t geom, uint16_t size, uint16_t io, uint8_t __far* desc)
{
    block_t blk = {};

    blk.id    = block_find_next_id(type);
    blk.type  = type;
    blk.sub   = sub;
    blk.lba   = lba;
    blk.geom  = geom;
    blk.size  = size;
    blk.io    = io;
    blk.flags = flags;

    blk.send_cmd = block_set_command(type);
    if (!blk.send_cmd)
        return false;

    if (!block_copy_desc(&blk, desc))
        return false;

    if (blk.type != BLOCK_TYPE_FLOPPY)
    {
        if (block_generate_fdpt(&blk) != BLOCK_SUCCESS)
            return false;
    }
    else
    {
        
    }

    if (!block_insert(&blk))
        return false;



    // debug_out("[BIOS] Created block device %02X\n\r", blk.id);

    return true;
}

uint8_t block_count(void)
{
    block_t __far* table = get_ebda()->block_table;
    uint8_t count = 0;

    for (uint8_t i = 0; i < 32; ++i)
    {
        if (block_valid(&table[i]))
            ++count;
    }

    return count;
}

uint8_t block_floppy_count(void)
{
    block_t __far* table = get_ebda()->block_table;
    uint8_t count = 0;

    for (uint8_t i = 0; i < 32; ++i)
    {
        if (block_valid(&table[i]) && table[i].id < 0x80)
            ++count;
    }

    return count;
}

uint8_t block_disk_count(void)
{
    block_t __far* table = get_ebda()->block_table;
    uint8_t count = 0;

    for (uint8_t i = 0; i < 32; ++i)
    {
        if (block_valid(&table[i]) && table[i].id >= 0x80)
            ++count;
    }

    return count;
}

uint8_t block_generate_fdpt(block_t __far* blk)
{
    uint16_t cylinders = blk->geom.cylinders;
    uint16_t heads     = blk->geom.heads;
    uint16_t sectors   = blk->geom.sectors;

    bool translated = false;

    if ((cylinders == 0 || heads == 0 || sectors == 0) && blk->type != BLOCK_TYPE_FLOPPY)
        translated = block_translate_lba(&blk->fdpt, blk->geom, blk->lba);
    else if (cylinders <= 1024 && heads <= 16 && sectors <= 63)
        translated = block_translate_none(&blk->fdpt, blk->geom);
    else if ((uint32_t)cylinders * heads <= 131072UL)
        translated = block_translate_chs(&blk->fdpt, blk->geom);
    else
        translated = block_translate_lba(&blk->fdpt, blk->geom, blk->lba);

    blk->flags = (uint8_t)(blk->flags | (translated ? BLOCK_XLATE : 0));
    blk->fdpt.signature = 0xA0;
    block_fdpt_create_checksum(&blk->fdpt);

    if (!block_fdpt_verify_checksum(&blk->fdpt))
        debug_out("[BIOS] FDPT checksum failed\n\r");
    
    return BLOCK_SUCCESS;
}

bool block_translate_none(fdpt_t __far* fdpt, geometry_t geom)
{
    if (geom.cylinders >= 1024) geom.cylinders = 1024;
    if (geom.heads >= 16)       geom.heads = 16;
    if (geom.sectors >= 63)     geom.sectors = 63;

    fdpt->physical_cylinders    = geom.cylinders - 1;
    fdpt->physical_heads        = (uint8_t)(geom.heads - 1);
    fdpt->physical_sectors      = (uint8_t)geom.sectors;
    fdpt->logical_cylinders     = geom.cylinders - 1;
    fdpt->logical_heads         = (uint8_t)(geom.heads - 1);
    fdpt->logical_sectors       = (uint8_t)geom.sectors;

    // debug_out("[BIOS] No translation: %d:%d:%d\n\r",
    //     fdpt->logical_cylinders, fdpt->logical_heads, fdpt->logical_sectors);

    return false;
}

bool block_translate_chs(fdpt_t __far* fdpt, geometry_t geom)
{
    uint16_t cyl = geom.cylinders;
    uint16_t hds = geom.heads;

    if (hds == 16)
    {
        if (cyl > 61439)
            cyl = 61439;

        hds = 15;
        cyl = (uint32_t)(cyl * 16) / 15;
    }

    while (cyl > 1024)
    {
        cyl >>= 1;
        hds <<= 1;

        if (hds > 127)
            break;
    }

    if (geom.cylinders >= 1024) geom.cylinders = 1023;
    if (geom.heads >= 256)      geom.heads = 255;
    if (geom.sectors >= 63)     geom.sectors = 63;

    fdpt->physical_cylinders    = geom.cylinders - 1;
    fdpt->physical_heads        = (uint8_t)(geom.heads - 1);
    fdpt->physical_sectors      = (uint8_t)geom.sectors;
    fdpt->logical_cylinders     = cyl - 1;
    fdpt->logical_heads         = (uint8_t)(hds - 1);
    fdpt->logical_sectors       = (uint8_t)geom.sectors;

    return true;
}

bool block_translate_lba(fdpt_t __far* fdpt, geometry_t geom, uint64_t lba)
{
    if (lba > (uint32_t)63 * 255 * 1024)
    {
        fdpt->physical_cylinders = 1023;
        fdpt->physical_heads    = 255;
        fdpt->physical_sectors  = 63;
        fdpt->logical_cylinders = 1023;
        fdpt->logical_heads     = 255;
        fdpt->logical_sectors   = 63;

        // debug_out("[BIOS] LBA translation: %d:%d:%d\n\r",
        //     fdpt->logical_cylinders, fdpt->logical_heads, fdpt->logical_sectors);

        return true;
    }

    uint32_t sectors = (uint32_t)lba / 63;

    uint32_t heads = (sectors > 1023L * 128) ? 255 :
                     (sectors > 1023L * 64)  ? 128 :
                     (sectors > 1023L * 32)  ? 64  :
                     (sectors > 1023L * 16)  ? 32  : 16;

    uint32_t cylinders = sectors / heads;

    if (geom.cylinders >= 1024) geom.cylinders = 1023;
    if (geom.heads >= 256)      geom.heads = 255;
    if (geom.sectors >= 63)     geom.sectors = 63;

    fdpt->physical_cylinders = geom.cylinders - 1;
    fdpt->physical_heads     = (uint8_t)(geom.heads - 1);
    fdpt->physical_sectors   = (uint8_t)geom.sectors;
    fdpt->logical_cylinders  = (uint16_t)cylinders - 1;
    fdpt->logical_heads      = (uint8_t)(heads - 1);
    fdpt->logical_sectors    = 63;

    // debug_out("[BIOS] LBA translation: %d:%d:%d\n\r",
    //     fdpt->logical_cylinders, fdpt->logical_heads, fdpt->logical_sectors);

    return true;
}

// bool block_find_fdpt(uint8_t id, fdpt_t __far*__far* fdpt)
// {
//     block_t __far* table = get_ebda()->block_table;
//     fdpt_t __far* fdpt_table = get_ebda()->fdpt;

//     for (uint8_t i = 0; i < 32; ++i)
//     {
//         if (table[i].id != id)
//             continue;

//         *fdpt = &fdpt_table[i];
//             return true;
//     }

//     *fdpt = NULL;
//     return false;
// }

// void block_get_fdpt(uint8_t index, fdpt_t __far*__far* fdpt)
// {
//     fdpt_t __far* fdpt_table = get_ebda()->fdpt;
//     *fdpt = &fdpt_table[index];
// }

void block_fdpt_create_checksum(fdpt_t __far* fdpt)
{
    uint8_t sum = 0;

    for (size_t i = 0; i < sizeof(fdpt_t); ++i)
        sum = (uint8_t)(sum + ((uint8_t __far*)(fdpt))[i]);

    fdpt->checksum = (uint8_t)-sum;
}

bool block_fdpt_verify_checksum(fdpt_t __far* fdpt)
{
    uint8_t sum = 0;

    for (size_t i = 0; i < sizeof(fdpt_t); ++i)
        sum = (uint8_t)(sum + ((uint8_t __far*)(fdpt))[i]);

    return sum == 0;
}

void block_untranslate_lchs(const block_t __far* blk,
    const geometry_t __far* lchs, uint32_t __far* lba)
{
    const fdpt_t __far* fdpt = &blk->fdpt;

    *lba = (uint32_t)(lchs->cylinders * fdpt->logical_heads +
        lchs->heads) * fdpt->logical_sectors + (lchs->sectors - 1);

    // uint32_t lba = (uint32_t)(cylinder * fdpt->logical_heads +
    //     head) * fdpt->logical_sectors + (sector - 1);

    // return lba;
}

void block_untranslate_pchs(const block_t __far* blk, 
    const uint32_t __far* lba, geometry_t __far* pchs)
{
    const fdpt_t __far* fdpt = &blk->fdpt;

    uint32_t temp = *lba % (uint32_t)(fdpt->physical_heads * fdpt->physical_sectors);
    pchs->cylinders = (uint16_t)(*lba  / (uint32_t)(fdpt->physical_heads * fdpt->physical_sectors));
    pchs->heads     = (uint16_t)((temp / fdpt->physical_sectors));
    pchs->sectors   = (uint16_t)((temp % fdpt->physical_sectors) + 1);
}

static geometry_t get_emulated_geom(uint8_t type)
{
    geometry_t geom = { 0, 0, 0 };

    if (type == 0x01)
    {
        geom.cylinders = 80;
        geom.heads     = 2;
        geom.sectors   = 15;
    }
    else if (type == 0x02)
    {
        geom.cylinders = 80;
        geom.heads     = 2;
        geom.sectors   = 18;
    }
    else if (type == 0x03)
    {
        geom.cylinders = 80;
        geom.heads     = 2;
        geom.sectors   = 36;
    }

    return geom;
}

uint8_t block_emulate_drive_floppy(block_t __far* blk, uint8_t type, uint32_t offset)
{
    block_t new = {};

    new.id    = 0x00;
    new.type  = BLOCK_TYPE_FLOPPY;
    new.sub   = blk->sub;
    new.flags = BLOCK_VALID | BLOCK_EMULA;
    new.geom  = get_emulated_geom(type);
    new.size  = 512;
    new.io    = blk->io;
    new.off   = offset;

    new.send_cmd = blk->send_cmd;

    if (!block_insert(&new))
        return BLOCK_ERROR;

    if (block_generate_fdpt(&new) != BLOCK_SUCCESS)
        return BLOCK_ERROR;

    return BLOCK_SUCCESS;
}

void block_list_all(void)
{
    block_t __far* table = get_ebda()->block_table;

    for (uint8_t i = 0; i < 32; ++i)
    {
        if (!block_valid(&table[i]))
            continue;

        debug_out("[BIOS] Block %02x: Type: %d, Sub: %d, LBA: %lu, Size: %d, IO: %d, Flags: %x, %d:%d:%d, Alt: %d\n\r",
            table[i].id, table[i].type, table[i].sub, (uint32_t)table[i].lba,
            table[i].size, table[i].io, table[i].flags,
            table[i].geom.cylinders, table[i].geom.heads, table[i].geom.sectors,
            table[i].alt);
    }
}

void block_list_all_fdpt(void)
{
    // fdpt_t __far* table = get_ebda()->fdpt;

    // for (uint8_t i = 0; i < 32; ++i)
    // {
    //     if (table[i].signature != 0xA0)
    //         continue;

    //     debug_out("[BIOS] FDPT %02x: %d:%d:%d, %d:%d:%d\n\r",
    //         i, table[i].logical_cylinders, table[i].logical_heads, table[i].logical_sectors,
    //         table[i].physical_cylinders, table[i].physical_heads, table[i].physical_sectors);
    // }
}