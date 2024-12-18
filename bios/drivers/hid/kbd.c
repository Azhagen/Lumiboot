#include "drivers/hid/kbd.h"
#include "drivers/hid/kbd_xt.h"
#include "drivers/hid/kbd_at.h"

#include "debug.h"
#include "system/system.h"
#include "system/data.h"
#include "utility.h"
#include "interrupt.h"
#include "debug.h"
#include "io.h"

#include <stddef.h>
#include <stdbool.h>

#include "data/keycode.h"

#define RSHIFT_DOWN     0x0001
#define LSHIFT_DOWN     0x0002
#define LCTRL_DOWN      0x0004
#define ALT_DOWN        0x0008
#define SCROLL_ACTIVE   0x0010
#define NUMLOCK_ACTIVE  0x0020
#define CAPS_ACTIVE     0x0040
#define INSERT_ACTIVE   0x0080
#define HOLD_ACTIVE     0x0800
#define SCROLL_DOWN     0x1000
#define NUMLOCK_DOWN    0x2000
#define CAPS_DOWN       0x4000
#define INSERT_DOWN     0x8000

#define NORM 0
#define SHFT 1
#define CTRL 2
#define ALT  3

void kbd_init(void)
{
    kbd_at_init();
}

uint8_t kbd_read_key(void)
{
    return kbd_at_read_key();
}

void kbd_set_leds(uint8_t leds)
{
    kbd_at_set_leds(leds);
}

void kbd_set_typematic(uint8_t value)
{
    kbd_at_set_typematic(value);
}

//static uint16_t process_code(uint8_t code);
// static bool handle_special(uint8_t code);
// static uint8_t set_flags(uint8_t code);

static uint8_t set_flags(uint8_t code);
static uint16_t process_code(uint8_t code);

static bool handle_special(uint8_t code)
{
    (void) code;
    return false;
}

void kbd_push_key(uint8_t code)
{
    code = set_flags(code);

    if (code & 0x80)
        return;

    if (handle_special(code))
        return;

    uint16_t value = process_code(code);

    uint16_t head = bda->keyboard_buffer_head - offsetof(bda_t, keyboard_buffer);
    uint16_t tail = bda->keyboard_buffer_tail - offsetof(bda_t, keyboard_buffer);

    bda->keyboard_buffer[head] = value;

    bda->keyboard_buffer_head = (head + 2) % 32 + offsetof(bda_t, keyboard_buffer);

    if (head == tail)
        bda->keyboard_buffer_tail = (tail + 2) % 32 + offsetof(bda_t, keyboard_buffer);

    // if (head >= 16)
    //     head = 0;

    // if (head == tail)
    //     tail = (head + 1 >= 16) ? 0 : head + 1;

    // bda->keyboard_buffer_head = head;
    // bda->keyboard_buffer_tail = tail;
}

static inline bool is_shift(void);
static inline bool is_caps(void);
static inline bool is_ctrl(void);
static inline bool is_alt(void);

static inline void set_rshift(bool value);
static inline void set_lshift(bool value);
static inline void set_ctrl(bool value);
static inline void set_alt(bool value);
static inline void set_scroll(bool value);
static inline void set_caps(bool value);
static inline void set_numlock(bool value);

static uint16_t process_code(uint8_t code)
{
    if (is_alt())
        return translation_table[code][ALT];

    if (is_ctrl())
        return translation_table[code][CTRL];

    if (is_shift() && !is_caps())
        return translation_table[code][SHFT];

    if (!is_shift() && is_caps())
        return translation_table[code][SHFT];

    if (is_shift() && is_caps())
        return translation_table[code][NORM];

    return translation_table[code][NORM];
}

static uint8_t set_flags(uint8_t code)
{
    // uint16_t flags = bda->keyboard_flags;

    switch (code)
    {
        case 0x1D: set_ctrl(true); break;
        case 0x9D: set_ctrl(false); break;

        case 0x2A: set_lshift(true); break;
        case 0xAA: set_lshift(false); break;
        
        case 0x36: set_rshift(true); break;
        case 0xB6: set_rshift(false); break;

        case 0x3A: set_caps(true); break;
        case 0xBA: set_caps(false); break;

        case 0x46: set_scroll(true); break;
        case 0xC6: set_scroll(false); break;

        case 0x45: set_numlock(true); break;
        case 0xC5: set_numlock(false); break;

        case 0x38: set_alt(true); break;
        case 0xB8: set_alt(false); break;

        // case 0x52: set_insert(true); break;
        // case 0x52: set_insert(false); break;

        default: break;
            //return code;
    }

    return code | (1 << 7);
}

static inline bool is_shift(void)
{
    return bda->keyboard_flags & (RSHIFT_DOWN | LSHIFT_DOWN);
}

static inline bool is_caps(void)
{
    return bda->keyboard_flags & CAPS_ACTIVE;
}

static inline bool is_ctrl(void)
{
    return bda->keyboard_flags & LCTRL_DOWN;
}

static bool is_alt(void)
{
    return bda->keyboard_flags & ALT_DOWN;
}

static void set_rshift(bool value)
{
    bda->rshift_pressed = value;
}

static void set_lshift(bool value)
{
    bda->lshift_pressed = value;
}

static void set_ctrl(bool value)
{
    bda->ctrl_pressed = value;
}

static void set_alt(bool value)
{
    bda->alt_pressed = value;
}

static void set_scroll(bool value)
{
    bda->scroll_pressed = value;
    bda->scroll_active  = (uint8_t)(bda->scroll_active ^ value);
}

static void set_caps(bool value)
{
    bda->capslock_pressed = value;
    bda->capslock_active  = (uint8_t)(bda->capslock_active ^ value);
}

static void set_numlock(bool value)
{
    bda->numlock_pressed = value;
    bda->numlock_active  = (uint8_t)(bda->numlock_active ^ value);
}