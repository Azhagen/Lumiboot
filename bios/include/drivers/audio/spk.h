#pragma once

#include <stdint.h>

void spk_beep(uint16_t msecs);
void spk_play_sound(uint16_t freq);
void spk_stop_sound(void);