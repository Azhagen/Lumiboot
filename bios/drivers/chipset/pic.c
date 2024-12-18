#include "drivers/chipset/pic.h"
#include "drivers/chipset/xt/pic.h"
#include "drivers/chipset/at/pic.h"

#include "io.h"
#include "system/data.h"
#include "system/system.h"

#define PIC1_CMD  0x20
#define PIC1_DATA 0x21
#define PIC2_CMD  0xA0
#define PIC2_DATA 0xA1

#define ICW4        0x01    /* ICW4 (not) needed */
#define SINGLE      0x02    /* Single (cascade) mode */
#define INTERVAL4   0x04    /* Call address interval 4 (8) */
#define LEVEL       0x08    /* Level triggered (edge) mode */
#define INIT        0x10    /* Initialization - required! */

#define I8086       0x01    /* 8086/88 (MCS-80/85) mode */
#define AUTO        0x02    /* Auto (normal) EOI */
#define BUF_SLAVE   0x08    /* Buffered mode/slave */
#define BUF_MASTER  0x0C    /* Buffered mode/master */
#define SFNM        0x10    /* Special fully nested (not) */

#define EOI 0x20

void pic_init(void)
{
    io_write(PIC1_CMD, INIT | ICW4);
    io_write(PIC2_CMD, INIT | ICW4);
    io_write(PIC1_DATA, 0x08);
    io_write(PIC2_DATA, 0x70);
    io_write(PIC1_DATA, 4);
    io_write(PIC2_DATA, 2);
    io_write(PIC1_DATA, I8086);
    io_write(PIC2_DATA, I8086);
}

void pic_enable_irq(uint8_t irq)
{
    if (irq >= 8)
        return;

    uint8_t mask = io_read(PIC1_DATA);
    mask &= (uint8_t)~(1 << irq);

    io_write(PIC1_DATA, mask);
}

void pic_disable_irq(uint8_t irq)
{
    if (irq >= 8)
        return;

    uint8_t mask = io_read(PIC1_DATA);
    mask |= (uint8_t)(1 << irq);
    
    io_write(PIC1_DATA, mask);
}

void pic_enable_all(void)
{
    io_write(PIC1_DATA, 0x00);
    io_write(PIC2_DATA, 0x00);
}

void pic_disable_all(void)
{
    io_write(PIC1_DATA, 0xFF);
    io_write(PIC2_DATA, 0xFF);
}

void pic_send_eoi(uint8_t irq)
{
    if (irq >= 8)
        io_write(PIC2_CMD, EOI);

    io_write(PIC1_CMD, EOI);
}