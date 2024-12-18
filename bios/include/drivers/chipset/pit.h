#pragma once

#include <stdint.h>

#define PIT_CHANNEL0 0
#define PIT_CHANNEL1 1
#define PIT_CHANNEL2 2

#define PIT_MODE0 0 // Interrupt on terminal count
#define PIT_MODE1 1 // Hardware re-triggerable one-shot
#define PIT_MODE2 2 // Rate generator
#define PIT_MODE3 3 // Square wave generator
#define PIT_MODE4 4 // Software triggered strobe
#define PIT_MODE5 5 // Hardware triggered strobe

void pit_init(void);
void pit_set_channel(uint8_t channel, uint8_t mode, uint16_t reload);