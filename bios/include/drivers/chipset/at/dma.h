#pragma once

#include <stdint.h>
#include "utility.h"

void dma_at_init(void);
void dma_at_reset(void);

void dma_at_set_addr(uint8_t channel, uint16_t addr);
void dma_at_set_size(uint8_t channel, uint16_t count);
void dma_at_set_page(uint8_t channel, uint8_t val);
void dma_at_set_mode(uint8_t channel, uint8_t mode);

void dma_at_mask_channel(uint8_t channel);
void dma_at_unmask_channel(uint8_t channel);
void dma_at_unmask_all(void);