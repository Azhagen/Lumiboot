#include "drivers/hid/kbd_at.h"
#include "drivers/misc/i8042.h"

#include "debug.h"
#include "system/data.h"

void kbd_at_init(void)
{
    i8042_init();
    i8042_init_kbd();
}

uint8_t kbd_at_read_key(void)
{
    return i8042_read_kbd();
}

void kbd_at_set_leds(uint8_t leds)
{

}

void kbd_at_set_typematic(uint8_t value)
{
    uint8_t ret = 0;

    do
    {
        i8042_write_kbd(0xF3);
        i8042_write_kbd(value);

        ret = i8042_read_kbd();

        if (ret == 0xFA)
            break;

    } while (ret == 0xFE);
}