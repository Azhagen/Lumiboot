#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "attrib.h"
#include "drivers.h"

struct setup_status
{
    uint16_t code;
    uint8_t  key;

    uint8_t  menu_id;
    uint8_t  prev_id;
    uint8_t  selected;

    // uint8_t  selected_0;
    // uint8_t  selected_1;
    // uint8_t  selected_2;

    // uint8_t  active_0;

    uint8_t  refresh_background;
    uint8_t  refresh;

    uint8_t  escape;

    time_t   time;
    date_t   date;
};

typedef struct setup_status setup_status_t;

void setup_main(void);
void setup_boot(void);
bool setup_wait_key(void);