#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "interrupt.h"
#include "utility.h"

static volatile pointer __far* const ivt = (pointer __far*)0x00000000L;

void interrupt_install(size_t num, uint16_t addr)
{
    ivt[num].seg = 0xF000;
    ivt[num].off = addr;
}

void interrupt_install_all(void)
{
    extern const uint16_t vtable[24];
    extern void interrupt_XXh(void);
    extern void interrupt_irq(void);
    extern void interrupt_70h(void);
    extern const struct floppy_parameter_table floppy_table;

    interrupt_install(0x00, (uint16_t)&interrupt_XXh);
    interrupt_install(0x01, (uint16_t)&interrupt_XXh);
    interrupt_install(0x02, (uint16_t)&interrupt_XXh);
    interrupt_install(0x03, (uint16_t)&interrupt_XXh);
    interrupt_install(0x04, (uint16_t)&interrupt_XXh);
    interrupt_install(0x05, (uint16_t)&interrupt_XXh);
    interrupt_install(0x06, (uint16_t)&interrupt_XXh);
    interrupt_install(0x07, (uint16_t)&interrupt_XXh);

    for (size_t i = 0; i < 24; ++i)
        interrupt_install(0x08 + i, vtable[i]);

    interrupt_install(0x70, (uint16_t)&interrupt_70h);
    
    for (size_t i = 0x71; i < 0x78; ++i)
        interrupt_install(i, (uint16_t)&interrupt_irq);
    
    interrupt_install(0x1E, (uint16_t)&floppy_table);
}