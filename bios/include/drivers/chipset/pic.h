#pragma once

#include <stdint.h>

void pic_init(void);

void pic_enable_irq(uint8_t irq);
void pic_disable_irq(uint8_t irq);

void pic_enable_all(void);
void pic_disable_all(void);

void pic_send_eoi(uint8_t irq);