#pragma once

#include <stdint.h>

void dma_xt_init(void);

void dma_xt_set_addr(uint8_t channel, uint16_t addr);
void dma_xt_set_size(uint8_t channel, uint16_t count);
void dma_xt_set_page(uint8_t channel, uint8_t val);
void dma_xt_set_mode(uint8_t channel, uint8_t mode);

void dma_xt_mask_channel(uint8_t channel);
void dma_xt_unmask_channel(uint8_t channel);
void dma_xt_unmask_all(void);
void dma_xt_reset(void);