#include "services/keyboard.h"
#include "drivers.h"

#include <stdbool.h>
#include "utility.h"
#include "system/data.h"
#include "debug.h"

#include "data/keycode.h"
#include "bios.h"

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

// static bool is_buffer_full(uint8_t head, uint8_t tail);
// static uint16_t read_status(void);
// static uint8_t  write_code(uint16_t code);

static void handle_key(uint8_t key);
static uint8_t intercept_key(uint8_t key);

static uint16_t read_code(void);
static uint16_t peek_code(void);
static uint16_t process_code(uint8_t code);
static uint8_t set_flags(uint8_t code);
static void init_keyboard(void);
static uint8_t store_code(uint16_t code);
// uint16_t kbd_peek_code(void);

void keyboard_push_key(void)
{
    uint8_t key = 0;
    key = kbd_read_key();
    key = intercept_key(key);
    handle_key(key);
    pic_send_eoi(1);
}

void keyboard_handler(intregs __seg_ss* const regs)
{
    switch (regs->ah)
    {
        case 0x00: keyboard_read_key(regs); break;
        case 0x01: keyboard_peek_key(regs); break;
        case 0x02: keyboard_get_flags(regs); break;
        case 0x03: keyboard_set_bits(regs); break;
        case 0x05: keyboard_store_key(regs); break;
        case 0x10: keyboard_ext_read_key(regs); break;
        case 0x11: keyboard_ext_peek_key(regs); break;
        case 0x12: keyboard_ext_get_flags(regs); break;

        default: break;
    }
}

void keyboard_read_key(intregs __seg_ss* const regs)
{
    regs->ax = read_code();
}

void keyboard_peek_key(intregs __seg_ss* const regs)
{
    regs->ax = peek_code();
    regs->ZF = regs->ax == 0;
}

void keyboard_get_flags(intregs __seg_ss* const regs)
{
    regs->al = lo(bda->keyboard_flags);
}

void keyboard_set_bits(intregs __seg_ss* const regs)
{
    if (regs->al != 0x05)
        return;

    uint8_t rate  = (uint8_t)((regs->bh << 5) & 0x60);
    uint8_t delay = (uint8_t)(regs->bl & 0x1F);

    kbd_set_typematic(rate | delay);
}

void keyboard_store_key(intregs __seg_ss* const regs)
{
    regs->al = store_code(regs->cx);
}

void keyboard_ext_read_key(intregs __seg_ss* const regs)
{
    keyboard_read_key(regs);
}

void keyboard_ext_peek_key(intregs __seg_ss* const regs)
{
    keyboard_peek_key(regs);
}

void keyboard_ext_get_flags(intregs __seg_ss* const regs)
{
    keyboard_get_flags(regs);
}

static void init_keyboard(void)
{
    bda->keyboard_buffer_head = offsetof(bda_t, keyboard_buffer);
    bda->keyboard_buffer_tail = offsetof(bda_t, keyboard_buffer);
    bda->keyboard_init = true;
}

// void keyboard_read_status(bioscall __seg_ss* const regs)
// {
//     uint16_t code = kbd_peek_code();
//     regs->ax = code;
//     regs->ZF = code == 0;

//     if (regs->ah != 0)
//         debug_out("key=%c\n\r", regs->ah);
// }

// void keyboard_read_flags(bioscall __seg_ss* const regs)
// {
//     regs->al = lo(bda->keyboard_flags);
// }

// void keyboard_store_key(bioscall __seg_ss* const regs)
// {
//     (void) regs;
//     // regs->al = write_code(as_uint16(regs->ch, regs->cl));
//     // regs->al = kbd_store_code(regs->ax);
// }

// static bool is_buffer_full(uint8_t head, uint8_t tail)
// {
//     return ((head + 1) % 16) == tail;
// }

static uint8_t store_code(uint16_t code)
{
    if (!bda->keyboard_init)
        init_keyboard();

    uint16_t head = bda->keyboard_buffer_head - offsetof(bda_t, keyboard_buffer);
    uint16_t tail = bda->keyboard_buffer_tail - offsetof(bda_t, keyboard_buffer);

    if ((head + 2) % 32 == tail)
        return 0;

    bda->keyboard_buffer[head / 2] = code;
    bda->keyboard_buffer_head = (head + 2) % 32 + offsetof(bda_t, keyboard_buffer);

    return 1;
}

static uint16_t peek_code(void)
{
    if (!bda->keyboard_init)
        init_keyboard();

    uint16_t head = bda->keyboard_buffer_head - offsetof(bda_t, keyboard_buffer);
    uint16_t tail = bda->keyboard_buffer_tail - offsetof(bda_t, keyboard_buffer);

    if (head == tail)
        return 0;

    return bda->keyboard_buffer[tail / 2];
}

static uint16_t read_code(void)
{
    if (!bda->keyboard_init)
        init_keyboard();

    uint16_t head = bda->keyboard_buffer_head - offsetof(bda_t, keyboard_buffer);
    uint16_t tail = bda->keyboard_buffer_tail - offsetof(bda_t, keyboard_buffer);

    while (head == tail)
    {
        asm volatile("hlt" ::: "memory");
        head = bda->keyboard_buffer_head - offsetof(bda_t, keyboard_buffer);
        tail = bda->keyboard_buffer_tail - offsetof(bda_t, keyboard_buffer);
        // debug_out("head=%x tail=%x\n\r", head, tail);
    }

    // debug_puts("Here\n\r");
    uint16_t code = bda->keyboard_buffer[tail / 2];
    bda->keyboard_buffer_tail = (tail + 2) % 32 + offsetof(bda_t, keyboard_buffer);

    return code;
}

static uint8_t intercept_key(uint8_t key)
{
    return bios_system_kbd_intercept(key);
}

static bool handle_special(uint8_t code)
{
    (void) code;
    return false;
    // if ()
}

static void handle_key(uint8_t key)
{
    if (!bda->keyboard_init)
        init_keyboard();

    key = set_flags(key);

    if (key & 0x80)
        return;

    if (handle_special(key))
        return;

    uint16_t code = process_code(key);

    uint16_t head = bda->keyboard_buffer_head - offsetof(bda_t, keyboard_buffer);
    uint16_t tail = bda->keyboard_buffer_tail - offsetof(bda_t, keyboard_buffer);

    bda->keyboard_buffer[head / 2] = code;
    head = (head + 2) % 32;
    bda->keyboard_buffer_head = head + offsetof(bda_t, keyboard_buffer);

    if (head == tail)
        bda->keyboard_buffer_tail = (tail + 2) % 32 + offsetof(bda_t, keyboard_buffer);
}

static bool is_shift(void);
static bool is_caps(void);
static bool is_ctrl(void);
static bool is_alt(void);

static void set_rshift(bool value);
static void set_lshift(bool value);
static void set_ctrl(bool value);
static void set_alt(bool value);
static void set_scroll(bool value);
static void set_caps(bool value);
static void set_numlock(bool value);

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

        default: return code;
            //return code;
    }

    return code | 0x80;
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
    bda->scroll_active ^= (uint16_t)value;
}

static void set_caps(bool value)
{
    bda->capslock_pressed = value;
    bda->capslock_active ^= (uint16_t)value;
}

static void set_numlock(bool value)
{
    bda->numlock_pressed = value;
    bda->numlock_active ^= (uint16_t)value;
}