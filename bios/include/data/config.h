#pragma once

#include <stdint.h>

typedef struct
{
    uint8_t size;
    uint8_t model;
    uint8_t submodel;
    uint8_t revision;
    uint8_t features_0;
    uint8_t features_1;
    uint8_t features_2;
    uint8_t features_3;
    uint8_t features_4;
} config_table;

const config_table config =
{
    sizeof(config_table),
    0xFF,
    0xFF,
    0x00,
    0x90,
    0x00,
    0x00,
    0x00,
    0x00
};