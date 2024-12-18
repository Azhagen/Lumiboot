#pragma once

#include "interrupt.h"
#include "attrib.h"

#include <stdint.h>

void kbd_init(void);
uint8_t kbd_read_key(void);

void kbd_set_leds(uint8_t leds);
void kbd_set_typematic(uint8_t value);