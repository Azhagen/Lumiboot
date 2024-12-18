#include "drivers/chipset/dma.h"
#include "drivers/chipset/xt/dma.h"
#include "drivers/chipset/at/dma.h"

#include "system/system.h"
#include "io.h"

#define DMA0_CHAN0_BASE 0x00
#define DMA0_CHAN1_BASE 0x02
#define DMA0_CHAN2_BASE 0x04
#define DMA0_CHAN3_BASE 0x06
#define DMA1_CHAN4_BASE 0xC0
#define DMA1_CHAN5_BASE 0xC4
#define DMA1_CHAN6_BASE 0xC8
#define DMA1_CHAN7_BASE 0xCC

#define DMA_CHAN0_PAGE 0x87
#define DMA_CHAN1_PAGE 0x83
#define DMA_CHAN2_PAGE 0x81
#define DMA_CHAN3_PAGE 0x82
#define DMA_CHAN4_PAGE 0x8F
#define DMA_CHAN5_PAGE 0x8B
#define DMA_CHAN6_PAGE 0x89
#define DMA_CHAN7_PAGE 0x8A

#define DMA0_STATUS         0x08
#define DMA0_COMMAND        0x08
#define DMA0_REQUEST        0x09
#define DMA0_CHANNEL_MASK   0x0a
#define DMA0_MODE           0x0b
#define DMA0_FLIPFLOP       0x0c
#define DMA0_TEMP           0x0d
#define DMA0_MASTER_RESET   0x0d
#define DMA0_MASK_RESET     0x0e
#define DMA0_MUTLI_MASK     0x0f

#define DMA1_MODE           0xd6
#define DMA1_CHANNEL_MASK   0xd4
#define DMA1_FLIPFLOP       0xd8
#define DMA1_MASK_RESET     0xde
#define DMA1_MASTER_RESET   0xda

static void dma_reset_ff(void);
static uint8_t dma_get_addr(uint8_t channel);
static uint8_t dma_get_count(uint8_t channel);
static uint8_t dma_get_page(uint8_t channel);

void dma_init(void)
{

}

void dma_set_addr(uint8_t channel, uint16_t addr)
{
    if (channel >= 8)
        return;

    uint16_t port = dma_get_addr(channel);

    dma_reset_ff();
    io_write(port, lo(addr));
    io_write(port, hi(addr));
}

void dma_set_size(uint8_t channel, uint16_t count)
{
    if (channel >= 8)
        return;

    uint16_t port = dma_get_count(channel);

    dma_reset_ff();
    io_write(port, lo(count));
    io_write(port, hi(count));
}

void dma_set_page(uint8_t channel, uint8_t val)
{
    if (channel >= 8)
        return;

    uint16_t port = dma_get_page(channel);
    io_write(port, val);
}

void dma_set_mode(uint8_t channel, uint8_t mode)
{
    if (channel >= 8)
        return;

    dma_mask_channel(channel);

    if (channel <= 4)
        io_write(DMA0_MODE, (uint8_t)(channel | mode));
    else
        io_write(DMA1_MODE, (uint8_t)(channel - 4) | mode);

    dma_unmask_channel(channel);
}

void dma_mask_channel(uint8_t channel)
{
    if (channel >= 8)
        return;
    
    if (channel < 4)
        io_write(DMA0_CHANNEL_MASK, (uint8_t)((1 << 2) | channel));
    else
        io_write(DMA1_CHANNEL_MASK, (uint8_t)((1 << 2) | (channel - 4)));
}

void dma_unmask_channel(uint8_t channel)
{
    if (channel >= 8)
        return;

    if (channel < 4)
        io_write(DMA0_CHANNEL_MASK, (uint8_t)(channel));
    else
        io_write(DMA1_CHANNEL_MASK, (uint8_t)(channel - 4));
}

void dma_unmask_all(void)
{
    io_write(DMA0_MASK_RESET, 0xff);
    io_write(DMA1_MASK_RESET, 0xff);
}

void dma_reset(void)
{
	io_write(DMA0_MASTER_RESET, 0xff);
    io_write(DMA1_MASTER_RESET, 0xff);
}

static void dma_reset_ff(void)
{
	io_write(DMA0_FLIPFLOP, 0xff);
    io_write(DMA1_FLIPFLOP, 0xff);
}

static uint8_t dma_get_addr(uint8_t channel)
{
    switch (channel) 
    {
        case 0: return DMA0_CHAN0_BASE;
        case 1: return DMA0_CHAN1_BASE;
        case 2: return DMA0_CHAN2_BASE;
        case 3: return DMA0_CHAN3_BASE;
        case 4: return DMA1_CHAN4_BASE;
        case 5: return DMA1_CHAN5_BASE;
        case 6: return DMA1_CHAN6_BASE;
        case 7: return DMA1_CHAN7_BASE;
        default: unreachable();
    }
}

static uint8_t dma_get_count(uint8_t channel)
{
    switch (channel) 
    {
        case 0: return DMA0_CHAN0_BASE + 1;
        case 1: return DMA0_CHAN1_BASE + 1;
        case 2: return DMA0_CHAN2_BASE + 1;
        case 3: return DMA0_CHAN3_BASE + 1;
        case 4: return DMA1_CHAN4_BASE + 2;
        case 5: return DMA1_CHAN5_BASE + 2;
        case 6: return DMA1_CHAN6_BASE + 2;
        case 7: return DMA1_CHAN7_BASE + 2;
        default: unreachable();
    }
}

static uint8_t dma_get_page(uint8_t channel)
{
    switch (channel)
    {
        case 0: return DMA_CHAN0_PAGE;
        case 1: return DMA_CHAN1_PAGE;
        case 2: return DMA_CHAN2_PAGE;
        case 3: return DMA_CHAN3_PAGE;
        case 4: return DMA_CHAN4_PAGE;
        case 5: return DMA_CHAN5_PAGE;
        case 6: return DMA_CHAN6_PAGE;
        case 7: return DMA_CHAN7_PAGE;
        default: unreachable();
    }
}