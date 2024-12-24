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

void boot_device(uint8_t id);
void boot_start(uint8_t id);