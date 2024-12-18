#include "data/system.h"

__section(".legacy.system")
const systable_t system_config =
{
    .size       = sizeof(systable_t),
    .model      = 0xFC,
    .submodel   = 0x00,
    .revision   = 0x00,
    .features   = 0x7C,
    .reserved   = {0x00, 0x00, 0x00, 0x00}
};