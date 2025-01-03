#include "data/floppy.h"
#include "attrib.h"

__section(".legacy.floppy") __align(1)
const floppy_parameter_table floppy_table =
{
    0xCF, 0x02, 0x25, 0x02, 0x08, 0x2A,
    0xFF, 0x50, 0xF6, 0x19, 0x04
};
