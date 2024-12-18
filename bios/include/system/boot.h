#pragma once

#include <stdint.h>
#include "attrib.h"

#include "system/block.h"

#define DEV_RESERVED    0x00
#define DEV_FLOPPY      0x01
#define DEV_HARDDISK    0x02
#define DEV_CDROM       0x03
#define DEV_PCMCIA      0x04
#define DEV_USBDEV      0x05
#define DEV_NETWORK     0x06
#define DEV_UNKNOWN     0xFF

// typedef void __far (*boot_handler_t)(void);

// struct __packed ipl_bcv_entry
// {
//     uint16_t device_type;
//     uint16_t status_flags;
//     boot_handler_t boot_handler;
//     void __far *description;
//     uint32_t reserved;
// };

// typedef struct ipl_bcv_entry boot_entry_t;
// // typedef struct ipl_bcv_entry bcv_entry_t;

// void boot_init(void);

// uint8_t boot_add_entry(uint8_t block_id);
// void boot_remove_entry(uint8_t block_id);

// uint8_t boot_get_entry(uint8_t index, boot_entry_t __far*__far* entry);

// // void boot_boot_device(uint8_t device);
// void boot_enable_emulation(uint8_t drive);