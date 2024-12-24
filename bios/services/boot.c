// #include "services/boot.h"

// #include <stdbool.h>

// #include "interrupt.h"
// #include "utility.h"
// #include "debug.h"
// #include "system/system.h"
// #include "system/data.h"

// #include "bios.h"
// #include "drivers.h"



// static boot_sector __far* const bootsect =
//     (boot_sector __far*)(0x00007c00L);

// extern noreturn void boot_start(uint8_t drive);

// void boot_floppy(uint8_t drive)
// {
//     /*
//     irq_enable();
    
//     bios_disk_reset(drive);

//     fdc_parameters params = { drive, 0, 0, 1, 1, 0x0000, 0x7c00 };
//     // uint8_t res = bios_disk_read_sectors(drive, 0, 0, 1, 1, bootsect);
//     uint8_t res = fdc_read_sectors(&params);

//     if (res != 0)
//     {
//         debug_puts("[BIOS] Boot: Failed to read floppy\n\r");
//         return;
//     }

//     if (bootsect->signature != 0xAA55)
//     {
//         debug_puts("[BIOS] Boot: Bad boot sector signature\n\r");
//         return;
//     }

//     debug_puts("[BIOS] Found boot disk...\n\r");

//     bios_video_set_mode(0x03);
//     bios_move_cursor(0, 0, 0);

//     boot_start(0);
//     */
// }

// void boot_no_device(void)
// {
//     // debug_out("[BIOS] No boot device found...\n\r");
    

//     while (true);
// }