#pragma once

#include <stdint.h>

void kbd_at_init();
uint8_t kbd_at_read_key();

void kbd_at_set_leds(uint8_t leds);
void kbd_at_set_typematic(uint8_t value);