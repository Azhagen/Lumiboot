#pragma once

#include <stdint.h>

struct __packed character
{
    uint8_t ch;
    uint8_t attr;
};

typedef struct character character_t;