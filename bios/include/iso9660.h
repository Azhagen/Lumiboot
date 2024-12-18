#pragma once

#include <stdint.h>
#include "attrib.h"

struct __packed boot_record
{
    uint8_t type;
    uint8_t identifier[5];
    uint8_t version;
    uint8_t system_id[32];
    uint8_t __reserved0[32];
    uint32_t lba_pointer;
};

struct __packed validation_entry
{
    uint8_t  header_id;
    uint8_t  platform_id;
    uint16_t __reserved;
    uint8_t  id_string[24];
    uint16_t checksum;
    uint16_t key;
};


struct __packed boot_entry
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

typedef struct boot_record      boot_record_t;
typedef struct validation_entry validation_entry_t;
typedef struct boot_entry       boot_entry_t;
