#pragma once

#include <stdint.h>
#include "attrib.h"

struct __packed system_config_table
{
    uint16_t size;
    uint8_t  model;
    uint8_t  submodel;
    uint8_t  revision;
    uint8_t  features;
    uint8_t  reserved[4];
};

typedef struct system_config_table systable_t;

extern const systable_t system_config;