#pragma once

#include <stdint.h>

typedef struct
{
    uint8_t specify_0;
    uint8_t specify_1;
    uint8_t motor_shutoff_count;
    uint8_t bytes_per_sector;
    uint8_t sectors_per_track;
    uint8_t gap_length;
    uint8_t data_length;
    uint8_t format_gap_length;
    uint8_t fill_byte;
    uint8_t head_settle_time;
    uint8_t motor_start_delay;
} floppy_parameter_table;

extern const floppy_parameter_table floppy_table;