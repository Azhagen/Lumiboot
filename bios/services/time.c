#include "services/time.h"
#include "system/data.h"

#include "drivers.h"

void time_handler(time_regs __seg_ss* const regs)
{
    switch (regs->ah)
    {
        case 0x00: time_read_clock(regs);   break;
        case 0x01: time_write_clock(regs);  break;
        case 0x02: time_read_time(regs);    break;
        case 0x03: time_write_time(regs);   break;
        case 0x04: time_read_date(regs);    break;
        case 0x05: time_write_date(regs);   break;
        case 0x06: time_set_alarm(regs);    break;
        case 0x07: time_reset_alarm(regs);  break;

        default: regs->CF = 1; break;
    }
}

void time_read_clock(time_regs __seg_ss* const regs)
{
    regs->cx = bda->timer_hi;
    regs->dx = bda->timer_lo;
    regs->al = bda->timer_of;
    
    bda->timer_of = 0;
    
    regs->CF = 0;
}

void time_write_clock(time_regs __seg_ss* const regs)
{
    bda->timer_hi = regs->cx;
    bda->timer_lo = regs->dx;
    bda->timer_of = 0;

    regs->CF = 0;
}

void time_read_time(time_regs __seg_ss* const regs)
{
    regs->CF = 1;

    if (cmos_read(0x0A) & 0x80)
        return;

    uint8_t seconds = cmos_read(0x00);
    uint8_t minutes = cmos_read(0x02);
    uint8_t hours   = cmos_read(0x04);
    uint8_t status  = cmos_read(0x0B);

    if (status & 0x04)
    {
        seconds = binary_to_bcd(seconds);
        minutes = binary_to_bcd(minutes);
        hours   = binary_to_bcd(hours);
    }

    cmos_enable_nmi();

    regs->ch = hours;
    regs->cl = minutes;
    regs->dh = seconds;
    regs->dl = status & 0x01;
    regs->CF = 0;
}

void time_write_time(time_regs __seg_ss* const regs)
{
    regs->CF = 1;

    if (cmos_read(0x0A) & 0x80)
        return;

    uint8_t seconds = regs->dh;
    uint8_t minutes = regs->cl;
    uint8_t hours   = regs->ch;
    uint8_t status  = cmos_read(0x0B);

    if (status & 0x04)
    {
        seconds = bcd_to_binary(seconds);
        minutes = bcd_to_binary(minutes);
        hours   = bcd_to_binary(hours);
    }

    cmos_write(0x00, seconds);
    cmos_write(0x02, minutes);
    cmos_write(0x04, hours);
    cmos_enable_nmi();
    regs->CF = 0;
}

void time_read_date(time_regs __seg_ss* const regs)
{
    regs->CF = 1;

    if (cmos_read(0x0A) & 0x80)
        return;

    uint8_t day     = cmos_read(0x07);
    uint8_t month   = cmos_read(0x08);
    uint8_t year    = cmos_read(0x09);
    uint8_t status  = cmos_read(0x0B);

    if (status & 0x04)
    {
        day     = binary_to_bcd(day);
        month   = binary_to_bcd(month);
        year    = binary_to_bcd(year);
    }

    cmos_enable_nmi();

    regs->ch = year;
    regs->cl = month;
    regs->dh = day;
    regs->dl = status & 0x01;
    regs->CF = 0;
}

void time_write_date(time_regs __seg_ss* const regs)
{
    regs->CF = 1;

    if (cmos_read(0x0A) & 0x80)
        return;

    uint8_t day     = regs->dh;
    uint8_t month   = regs->cl;
    uint8_t year    = regs->ch;
    uint8_t status  = cmos_read(0x0B);

    if (status & 0x04)
    {
        day     = bcd_to_binary(day);
        month   = bcd_to_binary(month);
        year    = bcd_to_binary(year);
    }

    cmos_write(0x07, day);
    cmos_write(0x08, month);
    cmos_write(0x09, year);
    cmos_enable_nmi();
    regs->CF = 0;
}

void time_set_alarm(time_regs __seg_ss* const regs)
{
    uint8_t seconds = regs->dh;
    uint8_t minutes = regs->cl;
    uint8_t hours   = regs->ch;
    uint8_t status  = cmos_read(0x0B);

    if (status & 0x04)
    {
        seconds = bcd_to_binary(seconds);
        minutes = bcd_to_binary(minutes);
        hours   = bcd_to_binary(hours);
    }

    cmos_write(0x01, seconds);
    cmos_write(0x03, minutes);
    cmos_write(0x05, hours);
    cmos_write(0x0B, cmos_read(0x0B) | 0x20);
    cmos_enable_nmi();
}

void time_reset_alarm(time_regs __seg_ss* const regs)
{
    (void) regs;
    cmos_write(0x0B, cmos_read(0x0B) & 0xDF);
    cmos_enable_nmi();
}