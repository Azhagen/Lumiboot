#pragma once

#include <stdint.h>

// void gfx_debug_init(void);
// void gfx_debug_tty_write(uint8_t ch, uint8_t attr);
// void gfx_debug_write(uint8_t ch, uint8_t attr, uint8_t x, uint8_t y);
// void gfx_debug_scroll(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t attr, int8_t lines);
// void gfx_debug_set_cursor_pos(uint8_t x, uint8_t y);

void debug_gfx_init(void);
void debug_gfx_tty_write(uint8_t ch, uint8_t attr);
void debug_gfx_write(uint8_t ch, uint8_t attr, uint8_t x, uint8_t y);
void debug_gfx_scroll(uint8_t x, uint8_t y, uint8_t width,
    uint8_t height, uint8_t attr, int8_t lines);
void debug_gfx_set_cursor_pos(uint8_t x, uint8_t y);