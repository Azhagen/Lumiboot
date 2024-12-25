#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdarg.h>

#include "drivers.h"

#include "utility.h"
#include "io.h"
#include "attrib.h"
#include "interrupt.h"

#include "debug.h"
#include "tui.h"
#include "bios.h"

#include "data/floppy.h"

#include "services/boot.h"
#include "services/keyboard.h"

#include "system/data.h"
#include "system/wait.h"
#include "system/system.h"
#include "system/system.h"

#include "setup/setup.h"

#include "print.h"
#include "string.h"

#include "iso9660.h"

#define DEBUG_PORT 0x80

const char version[]   = "Version 0.1.0 Alpha";
const char copyright[] = "Copyright (C) 2024 Azhagen";
const char license[]   = "Released under LGPLv3";

const char option_1[] = "F1: System setup menu";
const char option_2[] = "F2: Select boot device";


void splash_screen(void)
{
    tui_image(20, 7, &lumilogo);

    draw_text(40 - sizeof(version) / 2, 12, version, 0x07);
    draw_text(40 - sizeof(copyright) / 2, 13, copyright, 0x07);
    draw_text(40 - sizeof(license) / 2, 14, license, 0x07);

    draw_text(40 - sizeof(option_2) / 2, 17, option_1, 0x07);
    draw_text(40 - sizeof(option_2) / 2, 18, option_2, 0x07);

    if (pit_wait_until(5000, setup_wait_key))
    {
        if (bios_keyboard_read_key() == 0x3B00)
            setup_main();
        else
            setup_boot();
    }

    tui_clear(29, 17, sizeof(option_2), 2);
}

static volatile boot_sector __far* const bootsect =
    (boot_sector __far*)(0x00007c00L);

extern noreturn void boot_start(uint8_t drive);

void boot_cdrom(uint8_t drive)
{
    debug_out("[BIOS] Booting from CD-ROM, drive %x\n\r", drive);

    block_t __far* blk = 0;
    if (!block_find(drive, &blk))
    {
        debug_puts("[BIOS] Error finding block device\n\r");
        return;
    }

    // command_t cmd = { BLOCK_CMD_LBA_READ, 0x11, 1, (void __far*)0x7C00L };
    command_t cmd = {};
    cmd.cmd = BLOCK_CMD_LBA_READ;
    cmd.lba = 0x11;
    cmd.cnt = 1;
    cmd.buf = (void __far*)0x7C00L;

    debug_out("[BIOS] Reading boot record, lba %lu, count %u\n\r", (uint32_t)cmd.lba, cmd.cnt);

    if (blk->send_cmd(blk, &cmd) != BLOCK_SUCCESS)
    {
        debug_puts("[BIOS] Error reading boot sector\n\r");
        return;
    }

    boot_record_t __far* boot_record = (boot_record_t __far*)0x7C00L;

    uint32_t lba   = boot_record->lba_pointer;
    uint16_t count = 1;

    debug_out("[BIOS] Reading LBA %lu, count %u\n\r", lba, count);

    cmd.cmd = BLOCK_CMD_LBA_READ;
    cmd.lba = lba;
    cmd.cnt = count;
    cmd.buf = (void __far*)0x7C00L;

    if (blk->send_cmd(blk, &cmd) != BLOCK_SUCCESS)
    {
        debug_puts("[BIOS] Error reading boot sector\n\r");
        return;
    }

    // for (size_t i = 0; i < 2048; i += 16)
    // {
    //     debug_out("[BIOS] %04X: ", i);
    //     for (size_t j = 0; j < 16; ++j)
    //         debug_out("%02X ", ((uint8_t __far*)bootsect)[i + j]);
    //     debug_puts("\n\r");
    // }

    validation_entry_t __far* validation = (validation_entry_t __far*)0x00007C00L;

    if (validation->header_id != 0x01)
    {
        debug_puts("[BIOS] Invalid header ID\n\r");
        return;
    }

    boot_entry_t __far* boot = (boot_entry_t __far*)(validation + 1);

    debug_out("boot: %02X, %02X, %04X, %02X, %02X, %08lX\n\r",
        boot->indicator, boot->media_type, boot->load_segment, boot->system_type,
        boot->sector_count, boot->load_lba);

    if (boot->indicator != 0x88)
    {
        debug_puts("[BIOS] Invalid boot indicator\n\r");
        return;
    }

    lba = boot->load_lba;
    count = boot->sector_count;

    uint8_t type = boot->media_type;

    debug_out("[BIOS] Booting from LBA %lu, count %u\n\r", lba, count);

    cmd.cmd = BLOCK_CMD_LBA_READ;
    cmd.lba = lba;
    cmd.cnt = count;
    cmd.buf = (void __far*)0x7C00L;

    if (blk->send_cmd(blk, &cmd) != BLOCK_SUCCESS)
    {
        debug_puts("[BIOS] Error reading boot sector\n\r");
        return;
    }

    bool needs_emulation = (type >= 0x01 && type <= 0x04);

    if (needs_emulation)
    {
        debug_out("[BIOS] Emulating CD-ROM\n\r");
        block_emulate_drive_floppy(blk, type, lba);
    }

    block_list_all();

    boot_start(0x00);
}

void boot_disk(uint8_t drive)
{
    debug_out("[BIOS] Booting from drive %x\n\r", drive);

    if (bios_disk_read_sectors(drive, 0, 0, 1, 1, (void __far*)bootsect) != 0)
    {
        debug_puts("[BIOS] Error reading boot sector\n\r");
        return;
    }
    
    if (bootsect->signature != 0xAA55)
    {
        debug_puts("[BIOS] Invalid boot sector signature\n\r");
        return;
    }

    debug_puts("[BIOS] Found boot disk\n\r");

    bios_set_video_mode(0x03);
    bios_move_cursor(0, 0, 0);

    boot_start(drive);
}

void boot_handler(void)
{
    debug_puts("[BIOS] Booting...\n\r");
    irq_enable();

    boot_disk(0x80);
    boot_disk(0x81);

    boot_disk(0x00);
    boot_disk(0x01);
    boot_disk(0x02);
    boot_disk(0x03);
}

void fail_handler(void)
{
    debug_puts("[BIOS] No boot device found\n\r");
    bios_keyboard_read_key();
}

extern void atapi_detect(void);

void bda_init(void)
{
    fmemset_8(bda, 0, sizeof(bda_t));
}

void ebda_init(void)
{
    ebda_t __far* ebda = get_ebda();
    fmemset_8(ebda, 0, sizeof(ebda_t));
}

void bios_init(void)
{
    // Early initialization
    bda_init();
    cmos_init();

    debug_set_output(DBG_COM1);
    debug_init();
    debug_puts("[BIOS] Starting initialization...\n\r");

    // Early detection routines
    sys_detect();
    mem_detect();

    // Install interrupts vectors
    interrupt_install_all();

    // Initialization
    ebda_init();
#ifdef GDB_DEBUG
    gdb_init();
#endif
    kbd_init();
    pic_init();
    dma_init();
    pit_init();
    // rtc_init();
    rom_init();

    irq_enable();
    pic_enable_all();

    // Detection routines
    lpt_detect();
    fdc_detect();
    ata_detect();
    // atapi_detect();

    spk_beep(250);

    debug_puts("[BIOS] Initialization complete\n\r");

    block_list_all();
    
    bios_video_set_mode(0x03);
    splash_screen();

    while (true)
    {
        bios_bootstrap();
        // boot_manager();
    }
}