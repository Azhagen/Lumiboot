#pragma once

#include <stdint.h>
#include "attrib.h"

struct time
{
    uint8_t second;
    uint8_t minute;
    uint8_t hour;
};

struct date
{
    uint8_t day;
    uint8_t month;
    uint8_t year;
};

typedef struct time time_t;
typedef struct date date_t;

uint8_t cmos_read(uint8_t reg);
void cmos_write(uint8_t reg, uint8_t val);
void cmos_enable_nmi(void);

uint16_t cmos_read_memsize(void);
uint16_t cmos_read_extsize(void);

void cmos_read_time(time_t __far* const time);
void cmos_read_date(date_t __far* const date);

void cmos_write_time(const time_t __far* const time);
void cmos_write_date(const date_t __far* const date);

// void cmos_read_alarm(time_t __seg_ss* const time);
// void cmos_write_alarm(const time_t __seg_ss* const time);
// void cmos_enable_alarm(void);
// void cmos_disable_alarm(void);