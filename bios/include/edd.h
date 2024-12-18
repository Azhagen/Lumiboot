#pragma once

#include <stdint.h>
#include "attrib.h"

struct __packed edd_address_packet
{
    uint8_t  length;
    uint8_t  __reserved0;
    uint8_t  blocks;
    uint8_t  __reserved1;
    uint16_t offset;
    uint16_t segment;
    uint64_t lba;
};

struct __packed edd_result_buffer
{
    uint16_t size;
    uint16_t flags;
    uint32_t cylinders;
    uint32_t heads;
    uint32_t sectors_per_track;
    uint64_t sectors;
    uint16_t bytes_per_sector;
    void __far* edd_config;
};

typedef struct edd_address_packet edd_address_packet_t;
typedef struct edd_result_buffer edd_result_buffer_t;