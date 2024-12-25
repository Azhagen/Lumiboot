#include "drivers/chipset/pit.h"

#include "io.h"
#include "utility.h"
#include "debug.h"

#define PIT_DATA 0x40U
#define PIT_CMD  0x43U

void pit_set_channel(uint8_t channel, uint8_t mode, uint16_t reload)
{
    if (channel >= 3 || mode >= 8)
        return;

    uint8_t cmd = (uint8_t)((channel << 6) | (3 << 4) | (mode << 1));

    io_write(PIT_CMD, cmd);
    io_write(PIT_DATA + channel, lo(reload));
    io_write(PIT_DATA + channel, hi(reload));
}

void pit_init(void)
{
    pit_set_channel(PIT_CHANNEL0, PIT_MODE3, 0x00);
    pit_set_channel(PIT_CHANNEL2, PIT_MODE3, 0x00);
}