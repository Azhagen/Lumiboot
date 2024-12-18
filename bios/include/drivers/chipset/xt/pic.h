#pragma once

#include <stdint.h>

void pic_xt_init(void);

void pic_xt_enable_irq(uint8_t irq);
void pic_xt_disable_irq(uint8_t irq);

void pic_xt_enable_all(void);
void pic_xt_disable_all(void);

void pic_xt_send_eoi(uint8_t irq);