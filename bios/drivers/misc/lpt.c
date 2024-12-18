#include "drivers/misc/lpt.h"

#include <stddef.h>

#include "interrupt.h"
#include "utility.h"
#include "debug.h"

#include "io.h"
#include "system/data.h"

const uint16_t lpt_ports[] =
{
    0x3BC, 0x378, 0x278
};

void lpt_detect(void)
{
    for (uint8_t i = 0; i < 3; ++i)
        bda->lpt_timeout[i] = 100;

    uint8_t off = 0;

    // Using 8088_bios method

    for (uint8_t i = 0; i < 3; ++i)
    {
        // Test ioport
        io_write(lpt_ports[i], 0xA5);
        io_write(0xC0, 0xFF);
        if (io_read(lpt_ports[i]) != 0xA5)
            continue;

        // Exists, set bda
        bda->lpt_iobase[off++] = lpt_ports[i];
    }
}

uint8_t lpt_read_status(uint16_t port)
{
    return io_read(bda->lpt_iobase[port] + 1) ^ 0x48;
}

uint8_t lpt_initialize(uint16_t port)
{
    io_write(bda->lpt_iobase[port] + 2, 0x08);
    io_wait(10);
    io_write(bda->lpt_iobase[port] + 2, 0x0C);
    return lpt_read_status(port);
}

uint8_t lpt_print(uint16_t port, uint8_t val)
{
    io_write(bda->lpt_iobase[port], val);

    for (size_t i = 0; i < 10; ++i)
    {
        uint8_t status = io_read(bda->lpt_iobase[port] + 1);
        if (!(status & 0x80))
            return lpt_read_status(port);
    }

    return lpt_read_status(port) | 0x1;
}