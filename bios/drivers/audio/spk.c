#include "drivers/audio/spk.h"
#include "drivers/chipset/pit.h"

#include "system/wait.h"

#include "io.h"
#include "utility.h"
#include "debug.h"

#define TICK_RATE 1193182UL

void spk_play_sound(uint16_t freq)
{
    uint16_t reload = lo16(TICK_RATE / freq);
    pit_set_channel(PIT_CHANNEL2, PIT_MODE3, reload);

    uint8_t value = io_read(0x61);
    uint8_t test = value | 3;
    if (value != test)
        io_write(0x61, (uint8_t)test);
}

void spk_stop_sound(void)
{
    uint8_t value = io_read(0x61) & 0xFC;
    io_write(0x61, value);
}

void spk_beep(uint16_t msecs)
{
    spk_play_sound(500);
    pit_wait(msecs);
    spk_stop_sound();
}