#include <stdint.h>
#include "attrib.h"
#include "data/floppy.h"
#include "data/video.h"

extern void interrupt_08h(void);
extern void interrupt_09h(void);
extern void interrupt_0Eh(void);
extern void interrupt_10h(void);
extern void interrupt_11h(void);
extern void interrupt_12h(void);
extern void interrupt_13h(void);
extern void interrupt_14h(void);
extern void interrupt_15h(void);
extern void interrupt_16h(void);
extern void interrupt_17h(void);
extern void interrupt_18h(void);
extern void interrupt_19h(void);
extern void interrupt_1Ah(void);
extern void interrupt_1Bh(void);
extern void interrupt_1Ch(void);
extern void interrupt_1Dh(void);
extern void interrupt_irq(void);

__section(".legacy.vtable") __align(1)
const uint16_t vtable[24] =
{
    (uint16_t)&interrupt_08h,
    (uint16_t)&interrupt_09h,
    (uint16_t)&interrupt_irq,
    (uint16_t)&interrupt_irq,
    (uint16_t)&interrupt_irq,
    (uint16_t)&interrupt_irq,
    (uint16_t)&interrupt_0Eh,
    (uint16_t)&interrupt_irq,
    (uint16_t)&interrupt_10h,
    (uint16_t)&interrupt_11h,
    (uint16_t)&interrupt_12h,
    (uint16_t)&interrupt_13h,
    (uint16_t)&interrupt_14h,
    (uint16_t)&interrupt_15h,
    (uint16_t)&interrupt_16h,
    (uint16_t)&interrupt_17h,
    (uint16_t)&interrupt_18h,
    (uint16_t)&interrupt_19h,
    (uint16_t)&interrupt_1Ah,
    (uint16_t)&interrupt_1Bh,
    (uint16_t)&interrupt_1Ch,
    (uint16_t)&video_table,
    (uint16_t)&floppy_table,
    (uint16_t)0,
};