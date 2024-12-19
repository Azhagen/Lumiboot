#include "services/block.h"

#include "system/data.h"

#include "debug.h"
#include "edd.h"

void block_handler(registers_t __seg_ss* const regs)
{
    switch (regs->ah)
    {
        case 0x00: block_reset(regs); break;
        case 0x01: block_status(regs); break;
        case 0x02: block_read(regs); break;
        case 0x03: block_write(regs); break;
        case 0x04: block_verify(regs); break;
        case 0x05: block_format(regs); break;
        case 0x08: block_params(regs); break;
        case 0x15: block_type(regs); break;
        case 0x16: block_change(regs); break;

        case 0x41: block_ext_check(regs); break;
        case 0x42: block_ext_read(regs); break;
        case 0x43: block_ext_write(regs); break;
        case 0x44: block_ext_verify(regs); break;
        // case 0x45: block_ext_locking(regs); break;
        // case 0x46: block_ext_dskchg(regs); break;
        case 0x47: block_ext_seek(regs); break;
        case 0x48: block_ext_params(regs); break;
        // case 0x49: block_ext_status(regs); break;

        default:
            regs->ah = 0x01; regs->CF = 1; break;
    }
}

void block_reset(registers_t __seg_ss* const regs)
{
    block_t __far* blk = 0;
    uint8_t result = 0x01;

    if (!block_find(regs->dl, &blk))
        goto end;

    command_t cmd = {};
    cmd.cmd = BLOCK_CMD_RESET;
    result  = blk->send_cmd(blk, &cmd);

end:
    regs->ah = result;
    regs->CF = result != 0x00;
}

void block_status(registers_t __seg_ss* const regs)
{
    regs->ah = 0x00;
    regs->al = 0x00;
    // if (regs->dl < 0x80)
    //     floppy_status(regs);
    // else
    //     disk_status(regs);
}

void block_read(registers_t __seg_ss* const regs)
{
    block_t __far* blk = 0;
    uint8_t result = 0x01;

    if (!block_find(regs->dl, &blk))
        goto end;

    geometry_t chs = { as_uint16((regs->cl >> 6), regs->ch), regs->dh, regs->cl & 0x3F };

    command_t cmd = {};
    cmd.cmd = BLOCK_CMD_CHS_READ;
    cmd.chs = chs;
    cmd.cnt = regs->al;
    cmd.buf = segoff_to_fp(regs->es, regs->bx);

    result  = blk->send_cmd(blk, &cmd);

    // for (size_t i = 0; i < 16; ++i)
    // {
    //     for (size_t j = 0; j < 16; ++j)
    //         debug_out("%02X ", ((uint8_t __far*)cmd.buf)[i * 16 + j]);
    //     debug_puts("\n\r");
    // }

end:
    // debug_out("Result: %x\n\r", result);
    regs->ah = result;
    regs->CF = result != 0x00;
}

void block_write(registers_t __seg_ss* const regs)
{
    block_t __far* blk = 0;
    uint8_t result = 0x01;

    if (!block_find(regs->dl, &blk))
        goto end;

    geometry_t chs = { as_uint16((regs->cl >> 6), regs->ch), regs->dh, regs->cl & 0x3F };

    command_t cmd = {};
    cmd.cmd = BLOCK_CMD_CHS_WRITE;
    cmd.chs = chs;
    cmd.cnt = regs->al;
    cmd.buf = segoff_to_fp(regs->es, regs->bx);
    result  = blk->send_cmd(blk, &cmd);

end:
    regs->ah = result;
    regs->CF = result != 0x00;
}

void block_verify(registers_t __seg_ss* const regs)
{
    block_t __far* blk = 0;
    uint8_t result = 0x01;

    if (!block_find(regs->dl, &blk))
        goto end;

    geometry_t chs = { as_uint16((regs->cl >> 6), regs->ch), regs->dh, regs->cl & 0x3F };

    command_t cmd = {};
    cmd.cmd = BLOCK_CMD_CHS_VERIFY;
    cmd.chs = chs;
    cmd.cnt = regs->al;
    cmd.buf = segoff_to_fp(regs->es, regs->bx);
    result = blk->send_cmd(blk, &cmd);

end:
    regs->ah = result;
    regs->CF = result != 0x00;
}

void block_format(registers_t __seg_ss* const regs)
{
    block_t __far* blk = 0;
    uint8_t result = 0x01;

    if (!block_find(regs->dl, &blk))
        goto end;

    geometry_t chs = { as_uint16((regs->cl >> 6), regs->ch), regs->dh, regs->cl & 0x3F };

    command_t cmd = {};
    cmd.cmd = BLOCK_CMD_CHS_FORMAT;
    cmd.chs = chs;
    cmd.cnt = regs->al;
    cmd.buf = segoff_to_fp(regs->es, regs->bx);
    result = blk->send_cmd(blk, &cmd);

end:
    regs->ah = result;
    regs->CF = result != 0x00;
}

static void floppy_params(registers_t __seg_ss* const regs)
{
    block_t __far* blk = 0;
    uint8_t result = 0x01;

    if (!block_find(regs->dl, &blk))
        goto end;

    // TODO: implement this properly
    dkpt_t __far* dkpt = &blk->dkpt;
    regs->ch = dkpt->track_count;
    regs->cl = dkpt->sector_count;
    regs->dh = dkpt->head_count;
    regs->bx = 0;
    regs->ax = 0;
    regs->dl = block_floppy_count();

    pointer ptr = (pointer)(void __far*)dkpt;
    regs->es = ptr.seg;
    regs->di = ptr.off;

end:
    regs->ah = result;
    regs->CF = result != 0x00;
}

static void disk_params(registers_t __seg_ss* const regs)
{
    block_t __far* blk = 0;
    uint8_t result = 0x01;

    if (!block_find(regs->dl, &blk))
        goto end;

    fdpt_t __far* fdpt = &blk->fdpt;
    regs->ch = (uint8_t)(fdpt->logical_cylinders);
    regs->cl = (uint8_t)(((fdpt->logical_cylinders >> 2) & 0xC0) |
        (fdpt->logical_sectors & 0x3F));
    regs->dh = (uint8_t)(fdpt->logical_heads);
    regs->dl = block_disk_count();

    pointer ptr = (pointer)(void __far*)fdpt;
    regs->es = ptr.seg;
    regs->bx = ptr.off;
    result   = 0x00;

end:
    regs->ah = result;
    regs->CF = result != 0x00;
}

void block_params(registers_t __seg_ss* const regs)
{
    if (regs->dl < 0x80)
        floppy_params(regs);
    else
        disk_params(regs);

//     block_t __far* blk = 0;
//     uint8_t result = 0x01;

//     if (!block_find(regs->dl, &blk))
//         goto end;

//     if (blk->type == BLOCK_TYPE_ATAPI)
//         goto end;

//     fdpt_t __far* fdpt = &blk->fdpt;

//     uint16_t cylinders = (uint16_t)fdpt->logical_cylinders;
//     uint16_t heads     = (uint16_t)fdpt->logical_heads;
//     uint16_t sectors   = (uint16_t)fdpt->logical_sectors;

//     regs->ch = (uint8_t)(cylinders);
//     regs->cl = (uint8_t)(((cylinders >> 2) & 0xC0) | (sectors & 0x3F));
//     regs->dh = (uint8_t)(heads);
//     regs->dl = block_disk_count();
    
//     pointer ptr = (pointer)(void __far*)fdpt;
//     regs->es = ptr.seg;
//     regs->bx = ptr.off;
//     result   = 0x00;

// end:
//     regs->ah = result;
//     regs->CF = result != 0x00;
}

void block_type(registers_t __seg_ss* const regs)
{
    block_t __far* blk = 0;
    uint8_t result = 0x01;

    if (!block_find(regs->dl, &blk))
        goto end;

    switch (blk->type)
    {
        case BLOCK_TYPE_FLOPPY:
            result = (blk->geom.cylinders == 40) ? 0x01 : 0x02;
            break;
        default:
            result = 0x03;
            break;
    }

    if (result == 0x03)
    {
        uint32_t lba = blk->lba > 0xFFFFFFFF ? 0xFFFFFFFF : (uint32_t)blk->lba;
        regs->cx = hi16(lba);
        regs->dx = lo16(lba);
    }

end:
    regs->ah = result;
    regs->CF = 0;
}


void block_change(registers_t __seg_ss* const regs)
{
    uint8_t result = 0x01;
    if (regs->dl >= 0x80)
        goto end;

    block_t __far* blk = 0;
    if (!block_find(regs->dl, &blk))
        goto end;

    command_t cmd = {};
    cmd.cmd = BLOCK_CMD_CHANGE;
    result  = blk->send_cmd(blk, &cmd);

    debug_out("[BIOS] Change media: %x\n\r", result);

end:
    regs->ah = result;
    regs->CF = 1;
}

void block_ext_check(registers_t __seg_ss* const regs)
{
    if (regs->dl < 0x80)
    {
        regs->ah = 0x01;
        regs->CF = 1;
        return;
    }

    block_t __far* blk = 0;

    if (!block_find(regs->dl, &blk))
    {
        regs->CF = 1;
        return;
    }

    regs->ah = 0x21;
    regs->cx = (1 << 0); // | (1 << 2) | (blk->type == BLOCK_TYPE_ATAPI) ? (1 << 1) : 0;
    regs->bx = 0xAA55;
    regs->CF = 0;
}

void block_ext_read(registers_t __seg_ss* const regs)
{
    block_t __far* blk = 0;
    uint8_t result = 0x01;

    if (!block_find(regs->dl, &blk))
        goto end;

    edd_address_packet_t __far* packet = segoff_to_fp(regs->ds, regs->si);
    if (packet->length < sizeof(edd_address_packet_t))
        goto end;

    if (packet->blocks > 0x7F)
        goto end;

    command_t cmd = {};
    cmd.cmd = BLOCK_CMD_LBA_READ;
    cmd.flg = CMD_EXT;
    cmd.lba = packet->lba;
    cmd.cnt = packet->blocks;
    cmd.buf = segoff_to_fp(packet->segment, packet->offset);
    result  = blk->send_cmd(blk, &cmd);

end:
    regs->ah = result;
    regs->CF = result != BLOCK_SUCCESS;
}

void block_ext_write(registers_t __seg_ss* const regs)
{
    block_t __far* blk = 0;
    uint8_t result = 0x01;

    if (!block_find(regs->dl, &blk))
        goto end;

    edd_address_packet_t __far* packet = segoff_to_fp(regs->ds, regs->si);
    if (packet->length < sizeof(edd_address_packet_t))
        goto end;

    if (packet->blocks > 0x7F)
        goto end;

    command_t cmd = {};
    cmd.cmd = BLOCK_CMD_LBA_WRITE;
    cmd.flg = CMD_EXT;
    cmd.lba = packet->lba;
    cmd.cnt = packet->blocks;
    cmd.buf = segoff_to_fp(packet->segment, packet->offset);
    result  = blk->send_cmd(blk, &cmd);

end:
    regs->ah = result;
    regs->CF = result != BLOCK_SUCCESS;
}

void block_ext_verify(registers_t __seg_ss* const regs)
{
    block_t __far* blk = 0;
    uint8_t result = 0x01;

    if (!block_find(regs->dl, &blk))
        goto end;

    edd_address_packet_t __far* packet = segoff_to_fp(regs->ds, regs->si);
    if (packet->length < sizeof(edd_address_packet_t))
        goto end;

    if (packet->blocks > 0x7F)
        goto end;

    command_t cmd = {};
    cmd.cmd = BLOCK_CMD_LBA_VERIFY;
    cmd.flg = CMD_EXT;
    cmd.lba = packet->lba;
    cmd.cnt = packet->blocks;
    result  = blk->send_cmd(blk, &cmd);

end:
    regs->ah = result;
    regs->CF = result != BLOCK_SUCCESS;
}

void block_ext_locking(registers_t __seg_ss* const regs)
{
    (void) regs;
}

void block_ext_dskchg(registers_t __seg_ss* const regs)
{
    (void) regs;
}

void block_ext_seek(registers_t __seg_ss* const regs)
{
    block_t __far* blk = 0;
    uint8_t result = 0x01;

    if (!block_find(regs->dl, &blk))
        goto end;

    edd_address_packet_t __far* packet = segoff_to_fp(regs->ds, regs->si);
    if (packet->length < sizeof(edd_address_packet_t))
        goto end;

    command_t cmd = {};
    cmd.cmd = BLOCK_CMD_LBA_SEEK;
    cmd.flg = CMD_EXT;
    cmd.lba = packet->lba;
    result  = blk->send_cmd(blk, &cmd);

end:
    regs->ah = result;
    regs->CF = result != BLOCK_SUCCESS;
}

void block_ext_params(registers_t __seg_ss* const regs)
{
    block_t __far* blk = 0;
    uint8_t result = 0x01;

    if (!block_find(regs->dl, &blk))
        goto end;

    edd_result_buffer_t __far* buffer = segoff_to_fp(regs->ds, regs->si);

    if (buffer->size < 26)
        goto end;

    if (buffer->size >= 26 && buffer->size < 30)
        buffer->size = 26;
    else
        buffer->size = 30;

    buffer->flags = 0x00;
    buffer->cylinders = blk->geom.cylinders;
    buffer->heads = blk->geom.heads;
    buffer->sectors_per_track = blk->geom.sectors;
    buffer->sectors = blk->lba;
    buffer->bytes_per_sector = blk->size;

    if (buffer->size == 30)
        buffer->edd_config = 0;

end:
    regs->ah = result;
    regs->CF = result != BLOCK_SUCCESS;
}

void block_ext_status(registers_t __seg_ss* const regs)
{
    (void) regs;
}