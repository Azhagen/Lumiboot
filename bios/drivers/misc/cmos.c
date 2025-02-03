#include "drivers/misc/cmos.h"
#include "io.h"
#include "utility.h"
#include "debug.h"

void cmos_write(uint8_t reg, uint8_t val)
{
    io_write(0x70, reg | 0x80);
    io_write(0x71, val);
}

uint8_t cmos_read(uint8_t reg)
{
    io_write(0x70, reg | 0x80);
    return io_read(0x71);
}

void cmos_enable_nmi(void)
{
    io_write(0x70, io_read(0x70) & 0x7F);
    io_read(0x71);
}

void cmos_disable_nmi(void)
{
    io_write(0x70, io_read(0x70) | 0x80);
    io_read(0x71);
}

uint16_t cmos_read_memsize(void)
{
    uint16_t low  = cmos_read(0x15);
    uint16_t high = cmos_read(0x16);
    return (high << 8) | low;
}

uint16_t cmos_read_extsize(void)
{
    uint16_t low  = cmos_read(0x30);
    uint16_t high = cmos_read(0x31);
    return (high << 8) | low;
}

void cmos_read_time(time_t __far* const time)
{
    while (cmos_read(0x0A) & 0x80);

    uint8_t status = cmos_read(0x0B);

    if (status & 0x04)
    {
        time->second = cmos_read(0x00);
        time->minute = cmos_read(0x02);
        time->hour   = cmos_read(0x04);
    }
    else
    {
        time->second = bcd_to_binary(cmos_read(0x00));
        time->minute = bcd_to_binary(cmos_read(0x02));
        time->hour   = bcd_to_binary(cmos_read(0x04));
    }
}

void cmos_read_date(date_t __far* const date)
{
    while (cmos_read(0x0A) & 0x80);

    uint8_t status = cmos_read(0x0B);

    if (status & 0x04)
    {
        date->day   = cmos_read(0x07);
        date->month = cmos_read(0x08);
        date->year  = cmos_read(0x09);
    }
    else
    {
        date->day   = bcd_to_binary(cmos_read(0x07));
        date->month = bcd_to_binary(cmos_read(0x08));
        date->year  = bcd_to_binary(cmos_read(0x09));
    }
}

void cmos_write_time(const time_t __far* const time)
{
    while (cmos_read(0x0A) & 0x80);

    uint8_t status = cmos_read(0x0B);

    if (status & 0x04)
    {
        cmos_write(0x00, time->second);
        cmos_write(0x02, time->minute);
        cmos_write(0x04, time->hour);
    }
    else
    {
        cmos_write(0x00, binary_to_bcd(time->second));
        cmos_write(0x02, binary_to_bcd(time->minute));
        cmos_write(0x04, binary_to_bcd(time->hour));
    }
}

void cmos_write_date(const date_t __far* const date)
{
    while (cmos_read(0x0A) & 0x80);

    uint8_t status = cmos_read(0x0B);

    if (status & 0x04)
    {
        cmos_write(0x07, date->day);
        cmos_write(0x08, date->month);
        cmos_write(0x09, date->year);
    }
    else
    {
        cmos_write(0x07, binary_to_bcd(date->day));
        cmos_write(0x08, binary_to_bcd(date->month));
        cmos_write(0x09, binary_to_bcd(date->year));
    }
}

void cmos_init(void)
{
    if (cmos_checksum_valid())
        return;

    for (uint8_t i = 0x10; i < 0x3F; ++i)
        cmos_write(i, 0);

    cmos_checksum_compute();
    cmos_disable_nmi();
}

bool cmos_checksum_valid(void)
{
    uint16_t checksum = 0;
    for (uint8_t i = 0x10; i < 0x2D; ++i)
        checksum += cmos_read(i);

    uint16_t cmos = as_uint16(cmos_read(0x2E), cmos_read(0x2F));

    return checksum == cmos;
}

void cmos_checksum_compute(void)
{
    uint16_t checksum = 0;
    for (uint8_t i = 0x10; i < 0x2D; ++i)
        checksum += cmos_read(i);

    cmos_write(0x2E, hi(checksum));
    cmos_write(0x2F, lo(checksum));
}