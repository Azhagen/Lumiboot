#pragma once

#include <stdint.h>

void lpt_detect(void);

uint8_t lpt_read_status(uint16_t port);
uint8_t lpt_initialize(uint16_t port);
uint8_t lpt_print(uint16_t port, uint8_t val);
