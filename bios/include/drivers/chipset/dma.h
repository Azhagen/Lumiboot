#pragma once

#include <stdint.h>

#define DMA_CHANNEL0 0
#define DMA_CHANNEL1 1
#define DMA_CHANNEL2 2
#define DMA_CHANNEL3 3

#define DMA_MODE_SELF_TEST  0x00
#define DMA_MODE_READ_XFER  0x04
#define DMA_MODE_WRITE_XFER 0x08

#define DMA_MODE_AUTO  0x10
#define DMA_MODE_IDEC  0x20
#define DMA_MODE_MASK  0xc0

#define DMA_MODE_XFER_ONDEMAND  0x00
#define DMA_MODE_XFER_SINGLE    0x40
#define DMA_MODE_XFER_BLOCK     0x80
#define DMA_MODE_XFER_CASCADE   0xC0

#define DMA_READ    DMA_MODE_READ_XFER  | DMA_MODE_XFER_SINGLE
#define DMA_WRITE   DMA_MODE_WRITE_XFER | DMA_MODE_XFER_SINGLE

void dma_init(void);

void dma_set_addr(uint8_t channel, uint16_t addr);
void dma_set_size(uint8_t channel, uint16_t count);
void dma_set_page(uint8_t channel, uint8_t val);
void dma_set_mode(uint8_t channel, uint8_t mode);

void dma_mask_channel(uint8_t channel);
void dma_unmask_channel(uint8_t channel);
void dma_unmask_all(void);