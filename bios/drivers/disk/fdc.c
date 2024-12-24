#include "drivers/disk/fdc.h"
#include "drivers/chipset/dma.h"
#include "drivers/misc/cmos.h"

#include "data/floppy.h"
#include "system/data.h"
#include "system/system.h"
#include "debug.h"
#include "utility.h"
#include "print.h"

#include "system/wait.h"
#include "system/block.h"

// TODO: support monster fdc
#define FDC0_IOBASE 0x3f0

#define FDC_STATA 0
#define FDC_STATB 1
#define FDC_DOR   2
#define FDC_TDR   3
#define FDC_MSR   4
#define FDC_DSR   4
#define FDC_FIFO  5
#define FDC_DIR   7
#define FDC_CONF  7

#define FLOPPY_DOR  0x3f2
#define FLOPPY_MSR  0x3f4
#define FLOPPY_FIFO 0x3f5
#define FLOPPY_CTRL 0x3f7

#define FDC_DMA_CHANNEL 2
#define FDC_CCR_TX_500  0x00
#define FDC_READ_CMD    FDC_CMD_READ_SECT   | FDC_CMD_EXT_MULTITRACK | FDC_CMD_EXT_DENSITY
#define FDC_WRITE_CMD   FDC_CMD_WRITE_SECT  | FDC_CMD_EXT_MULTITRACK | FDC_CMD_EXT_DENSITY
#define DMA_READ_CH2    DMA_MODE_READ_XFER  | DMA_MODE_XFER_SINGLE   | 2
#define DMA_WRITE_CH2   DMA_MODE_WRITE_XFER | DMA_MODE_XFER_SINGLE   | 2
#define DMA_VERIFY_CH2  DMA_MODE_SELF_TEST  | DMA_MODE_XFER_SINGLE   | 2

static bool fdc_is_write_ready(uint16_t iobase)
{
    return (io_read(iobase + FDC_MSR) & 0x80) == 0x80;
}

static bool fdc_is_read_ready(uint16_t iobase)
{
    return (io_read(iobase + FDC_MSR) & 0x80) == 0x80;
}

static void fdc_enable_controller(uint16_t iobase)
{
    io_write(iobase + FDC_DOR, 0x0C);
}

static void fdc_disable_controller(uint16_t iobase)
{
    io_write(iobase + FDC_DOR, 0x00);
}

static uint8_t fdc_read_data(uint16_t iobase)
{
    while (!fdc_is_read_ready(iobase));
    return io_read(iobase + FDC_FIFO);
}

static bool fdc_test_irq(void)
{
    if (bda->floppy_recalibration & 0x80) {
        bda->floppy_recalibration &= (uint8_t)(~0x80);
        return true;
    }

    return false;
}

static bool fdc_wait_irq(void)
{
    return pit_wait_until(1000, fdc_test_irq);
}

static bool fdc_dma_setup(uint16_t segment, uint16_t offset, uint16_t count, uint8_t mode)
{
    uint32_t value = segoff_to_linear(offset, segment);
    uint16_t addr = lo16(value);
    uint16_t size = (count << 9) - 1;
    uint8_t  page = lo(hi16(value) + (segment >> 12));

    if (addr > 0xFFFF - size) return false;

    irq_disable();
    dma_mask_channel(FDC_DMA_CHANNEL);
    dma_set_page(FDC_DMA_CHANNEL, page);
    dma_set_addr(FDC_DMA_CHANNEL, addr);
    dma_set_size(FDC_DMA_CHANNEL, size);
    dma_set_mode(FDC_DMA_CHANNEL, mode);
    dma_unmask_channel(FDC_DMA_CHANNEL);
    irq_enable();

    return true;
}

static uint8_t fdc_set_error(uint8_t st0, uint8_t st1, uint8_t st2)
{
    if (st0 & 0x08) return FLOPPY_ERR_TIMEOUT;
    if (st1 & 0x01) return BLOCK_ERR_ADDR_MARK;
    if (st1 & 0x02) return FLOPPY_ERR_PROTECTED;
    if (st1 & 0x04) return FLOPPY_ERR_SECTOR;
    if (st1 & 0x10) return FLOPPY_ERR_OVERRUN;
    if (st1 & 0x20) return FLOPPY_ERR_CRC_ECC;
    if (st1 & 0x80) return FLOPPY_ERR_MEDIA;
    return FLOPPY_NO_ERROR;
}

static uint8_t fdc_read_response(uint16_t iobase)
{
    for (uint8_t i = 0; i < 7; ++i)
    {
        uint8_t val = fdc_read_data(iobase);
        // debug_out("FDC: Response %d: %x\r\n", i, val);
        bda->floppy_fdc_status[i] = val;
    }

    return fdc_set_error(bda->floppy_fdc_status[0],
        bda->floppy_fdc_status[1], bda->floppy_fdc_status[2]);
}

static void fdc_send_command(uint16_t iobase, uint8_t cmd)
{
    while (!fdc_is_write_ready(iobase));
    io_write(iobase + FDC_FIFO, cmd);
}

static void fdc_check_int(uint16_t iobase)
{
    fdc_send_command(iobase, FDC_CMD_CHECK_INT);
    bda->floppy_fdc_status[0] = fdc_read_data(iobase);
    bda->floppy_fdc_status[3] = fdc_read_data(iobase);
    // debug_out("FDC: Status %x, Cylinder %d\r\n", bda->floppy_fdc_status[0], bda->floppy_fdc_status[3]);
}

static void fdc_cmd_specify(uint16_t iobase, uint8_t stepr,
    uint8_t loadt, uint8_t unloadt)
{
    fdc_send_command(iobase, FDC_CMD_SPECIFY);
    fdc_send_command(iobase, (uint8_t)(((stepr & 0xf) << 4) | (unloadt & 0xf)));
    fdc_send_command(iobase, (uint8_t)(loadt << 1));
}

static void fdc_select_drive(uint16_t iobase, uint8_t drive)
{
    bda->floppy_motor |= (uint8_t)(1 << drive);
    bda->floppy_motor_shutoff = 0xFF;

    io_write(iobase + FDC_DOR, (uint8_t)(1 << (drive + 4)) | 0x0C | drive);
}

uint8_t fdc_seek_absolute(uint16_t iobase, 
    uint8_t drive, uint8_t cylinder, uint8_t head)
{
    fdc_send_command(iobase, FDC_CMD_SEEK);
    fdc_send_command(iobase, (uint8_t)((head) << 2) | drive);
    fdc_send_command(iobase, cylinder);

    if (!fdc_wait_irq()) return BLOCK_ERR_TIMEOUT;

    fdc_check_int(iobase);

    // debug_out("FDC: Cylinder %d, Status %x\r\n", bda->floppy_fdc_status[3], bda->floppy_fdc_status[0]);

    if ((bda->floppy_fdc_status[0] & 0xC0) != 0x00 ||
        bda->floppy_fdc_status[3] != cylinder)
        return BLOCK_ERR_SEEK;
        
    return BLOCK_SUCCESS;
}

static uint8_t fdc_cmd_recalibrate(uint16_t iobase, uint8_t drive)
{
    fdc_send_command(iobase, FDC_CMD_RECALIBRATE);
    fdc_send_command(iobase, drive);

    if (!fdc_wait_irq())
        return BLOCK_ERR_TIMEOUT;

    fdc_check_int(iobase);
    
    if ((bda->floppy_fdc_status[0] & 0xC0) != 0x00 ||
        bda->floppy_fdc_status[3] != 0x00)
        return BLOCK_ERR_SEEK;

    return BLOCK_SUCCESS;
}

static uint8_t fdc_cmd_drive_status(uint16_t iobase, uint8_t drive, uint8_t head)
{
    fdc_send_command(iobase, FDC_CMD_CHECK_STAT);
    fdc_send_command(iobase, (uint8_t)((head << 2) | drive));
    return fdc_read_data(iobase);
}

static uint8_t fdc_reset_controller(uint16_t iobase)
{
    fdc_disable_controller(iobase);
    pit_wait(100);
    fdc_enable_controller(iobase);
    
    if (!fdc_wait_irq())
        return BLOCK_ERR_TIMEOUT;

    for (uint8_t i = 0; i < 4; ++i)
        fdc_check_int(iobase);

    return BLOCK_SUCCESS;
}

static uint8_t fdc_cmd_read_id(uint16_t iobase, uint8_t drive, uint8_t head)
{
    fdc_send_command(iobase, FDC_CMD_READ_ID_S | FDC_CMD_EXT_DENSITY);
    fdc_send_command(iobase, (uint8_t)((head << 2) | drive));

    if (!fdc_wait_irq())
        return BLOCK_ERR_TIMEOUT;

    return fdc_read_response(iobase);
}


static uint8_t fdc_cmd_rwv(block_t __far* blk, geometry_t __seg_ss* chs,
    uint16_t count, void __far* buffer, uint8_t fdc_cmd, uint8_t dma_cmd)
{
    uint8_t cylinder = (uint8_t)chs->cylinders;
    uint8_t head     = (uint8_t)chs->heads;
    uint8_t sector   = (uint8_t)chs->sectors;

    fdc_select_drive(blk->io, blk->sub);

    if (blk->flags & BLOCK_RECAL)
    {
        debug_out("[BIOS] FDC: Recalibrating drive %d\n\r", blk->sub);
        if (fdc_cmd_recalibrate(blk->io, blk->sub) != BLOCK_SUCCESS)
            return BLOCK_ERROR;

        blk->flags &= (uint8_t)(~BLOCK_RECAL);
    }

    if (!(blk->flags & BLOCK_MEDIA))
    {
        debug_out("[BIOS] FDC: Sense media\n\r");
        if (fdc_sense_media(blk) != BLOCK_SUCCESS)
            return BLOCK_ERROR;
    }

    pointer ptr = (pointer)buffer;
    if (!fdc_dma_setup(ptr.seg, ptr.off, count, dma_cmd))
        return FLOPPY_ERR_BOUNDARY;

    if (fdc_seek_absolute(blk->io, blk->sub, cylinder, head) != BLOCK_SUCCESS)
        return FLOPPY_ERR_SEEK;

    uint8_t end = (uint8_t)((sector + count) >= FLOPPY_SECTORS_PER_TRACK
        ? FLOPPY_SECTORS_PER_TRACK : sector + count);
    uint8_t drv = (uint8_t)((head << 2) | blk->sub);

    fdc_send_command(blk->io, fdc_cmd);
    fdc_send_command(blk->io, drv);
    fdc_send_command(blk->io, cylinder);
    fdc_send_command(blk->io, head);
    fdc_send_command(blk->io, sector);
    fdc_send_command(blk->io, FLOPPY_SECTOR_DTL_512);
    fdc_send_command(blk->io, end);
    fdc_send_command(blk->io, FLOPPY_GAP3_LENGTH_3_5);
    fdc_send_command(blk->io, 0xFF);

    if (!fdc_wait_irq())
        return FLOPPY_ERR_TIMEOUT;

    return fdc_read_response(blk->io);
}

void fdc_geom_from_type(uint8_t type, geometry_t __seg_ss* geom)
{
    switch (type)
    {
        case 1: *geom = (geometry_t){ 40, 2, 9  }; return;
        case 2: *geom = (geometry_t){ 80, 2, 15 }; return;
        case 3: *geom = (geometry_t){ 80, 2, 9  }; return;
        case 4: *geom = (geometry_t){ 80, 2, 18 }; return;
        case 5: *geom = (geometry_t){ 80, 2, 36 }; return;
        default: unreachable();
    }
}

void fdc_detect(void)
{
    uint16_t iobase = FDC0_IOBASE;

    uint8_t count = 0;
    uint8_t floppy_a = (cmos_read(0x10) & 0xF0) >> 4;
    uint8_t floppy_b = (cmos_read(0x10) & 0x0F) >> 0;

    if (fdc_reset_controller(iobase) != BLOCK_SUCCESS)
        return;

    if (floppy_a > 0 && floppy_a <= 5)
    {
        uint8_t drive = 0;
        geometry_t geom = {};
        fdc_geom_from_type(floppy_a, &geom);
        fdc_select_drive(iobase, drive);
        if (fdc_cmd_recalibrate(iobase, drive) != BLOCK_SUCCESS) return;
        if (fdc_cmd_recalibrate(iobase, drive) != BLOCK_SUCCESS) return;
        block_create(BLOCK_TYPE_FLOPPY, count, 0, 0,
            geom, 512, iobase, to_fp("Floppy A"));
        count++;
    }

    if (floppy_b > 0 && floppy_b <= 5)
    {
        uint8_t drive = 1;
        geometry_t geom = {};
        fdc_geom_from_type(floppy_b, &geom);
        fdc_select_drive(iobase, drive);
        if (fdc_cmd_recalibrate(iobase, drive) != BLOCK_SUCCESS) return;
        if (fdc_cmd_recalibrate(iobase, drive) != BLOCK_SUCCESS) return;
        block_create(BLOCK_TYPE_FLOPPY, count, 0, 0,
            geom, 512, iobase, to_fp("Floppy B"));
        count++;
    }

    bda->equipment_list |= (uint16_t)((count - 1) << 6);
}

uint8_t fdc_send_cmd(block_t __far* blk, command_t __seg_ss* cmd)
{
    uint8_t count = (uint8_t)cmd->cnt;
    
    switch (cmd->cmd)
    {
        case BLOCK_CMD_RESET:
            return fdc_reset(blk);
        case BLOCK_CMD_CHS_READ:
            return fdc_read_chs(blk, &cmd->chs, count, cmd->buf);
        case BLOCK_CMD_CHS_WRITE:
            return fdc_write_chs(blk, &cmd->chs, count, cmd->buf);
        case BLOCK_CMD_CHS_VERIFY:
            return fdc_verify_chs(blk, &cmd->chs, count);
        case BLOCK_CMD_CHS_FORMAT:
            return fdc_format_chs(blk, &cmd->chs, count, cmd->buf);
        case BLOCK_CMD_CHANGE:
            return fdc_check_media(blk);
            
        default: break;
    }

    return BLOCK_ERROR;
}

uint8_t fdc_reset(block_t __far* blk)
{
    uint8_t res = 0;
    if ((res = fdc_reset_controller(blk->io)) != BLOCK_SUCCESS)
        return res;

    fdc_cmd_specify(blk->io, 0x0F, 0x02, 0x0F);

    blk->flags |= BLOCK_RECAL;
    return BLOCK_SUCCESS;
}

uint8_t fdc_sense_media(block_t __far* blk)
{
    static const uint8_t rates[] = { 0, 1, 2, 3 }; // 500kbps, 300kbps, 250kbps, 1000kbps

    for (size_t i = 0; i < array_size(rates); ++i)
    {
        fdc_select_drive(blk->io, blk->sub);

        debug_out("[BIOS] FDC: Drive %d: Testing rate %d\n\r", blk->sub, rates[i]);

        io_write(blk->io + FDC_CONF, rates[i]);

        if (fdc_cmd_read_id(blk->io, blk->sub, 0) != BLOCK_SUCCESS)
            continue;

        debug_out("[BIOS] FDC: Drive %d: Media detected, rate %d\n\r", blk->sub, rates[i]);

        blk->flags |= BLOCK_MEDIA;
        return BLOCK_SUCCESS;
    }

    return BLOCK_ERR_MEDIA;
}

uint8_t fdc_check_media(block_t __far* blk)
{
    fdc_select_drive(blk->io, blk->sub);

    if (io_read(blk->io + FDC_DIR) & 0x80)
    {
        blk->flags &= (uint8_t)(~BLOCK_MEDIA);
        return BLOCK_ERR_MEDIA;
    }
    else
        return BLOCK_SUCCESS;
}

uint8_t fdc_read_chs(block_t __far* blk, geometry_t __seg_ss* chs,
    uint16_t count, void __far* buffer)
{
    return fdc_cmd_rwv(blk, chs, count, buffer, FDC_CMD_READ_SECT |
        FDC_CMD_EXT_MULTITRACK | FDC_CMD_EXT_DENSITY, DMA_READ_CH2);
}

uint8_t fdc_write_chs(block_t __far* blk, geometry_t __seg_ss* chs,
    uint8_t count, void __far* buffer)
{
    return fdc_cmd_rwv(blk, chs, count, buffer, FDC_CMD_WRITE_SECT |
        FDC_CMD_EXT_MULTITRACK | FDC_CMD_EXT_DENSITY, DMA_WRITE_CH2);
}

uint8_t fdc_verify_chs(block_t __far* blk, geometry_t __seg_ss* chs, uint8_t count)
{
    return fdc_cmd_rwv(blk, chs, count, NULL, FDC_CMD_READ_SECT |
        FDC_CMD_EXT_MULTITRACK | FDC_CMD_EXT_DENSITY, DMA_VERIFY_CH2);
}

uint8_t fdc_format_chs(block_t __far* blk, geometry_t __seg_ss* chs,
    uint8_t count, void __far* buffer)
{
    (void)blk;
    (void)chs;
    (void)count;
    (void)buffer;
    return BLOCK_ERROR;
}