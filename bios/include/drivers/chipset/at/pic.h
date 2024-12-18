#pragma once

#include <stdint.h>

void pic_at_init(void);

void pic_at_enable_irq(uint8_t irq);
void pic_at_disable_irq(uint8_t irq);

void pic_at_enable_all(void);
void pic_at_disable_all(void);

void pic_at_send_eoi(uint8_t irq);