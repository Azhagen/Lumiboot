#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "attrib.h"

#define ALIGN_CENTER    0x00
#define ALIGN_LEFT      0x01
#define ALIGN_RIGHT     0x02

#define BORDER_OMIT     0x00
#define BORDER_LARGE    0x01
#define BORDER_SIMPLE   0x02

#define COLOR_BLACK     0x00
#define COLOR_BLUE      0x01
#define COLOR_GREEN     0x02
#define COLOR_CYAN      0x03
#define COLOR_RED       0x04
#define COLOR_MAGENTA   0x05
#define COLOR_BROWN     0x06
#define COLOR_LIGHTGRAY 0x07
#define COLOR_DARKGRAY  0x08
#define COLOR_LIGHTBLUE 0x09
#define COLOR_LIGHTGREEN 0x0A
#define COLOR_LIGHTCYAN 0x0B
#define COLOR_LIGHTRED  0x0C
#define COLOR_LIGHTMAGENTA 0x0D
#define COLOR_YELLOW    0x0E
#define COLOR_WHITE     0x0F

typedef struct
{
    uint8_t version;
    uint8_t width;
    uint8_t height;
    uint8_t data[];
} logo;

extern const logo lumilogo;

static inline uint8_t make_attr(uint8_t fg, uint8_t bg)
{
    return (bg << 4) | fg;
}

void draw_text(uint8_t x, uint8_t y, const char __far* str, uint8_t attr);
void draw_textbox(uint8_t x, uint8_t y, uint8_t w, uint8_t h,
    const char __far* str, size_t size, uint8_t attr, uint8_t border, uint8_t align);

void tui_image(uint8_t x, uint8_t y, const logo __far* img);
void tui_clear(uint8_t x, uint8_t y, uint8_t width, uint8_t height);

void draw_border(uint8_t x, uint8_t y, uint8_t w, uint8_t h,
    uint8_t attr, uint8_t border);

void draw_tabs(uint8_t x, uint8_t y, uint8_t w, uint8_t count,
    uint8_t selected, uint8_t attr, uint8_t hi_attr, const char* const items[]);

void fill_region(uint8_t x, uint8_t y, uint8_t w,
    uint8_t h, uint8_t c, uint8_t fg, uint8_t bg);

void tui_menulist(uint8_t x, uint8_t y, uint8_t w, uint8_t h,
    uint8_t attr, uint8_t hi_attr, uint8_t selected,
    uint8_t count, const char* const items[]);

void draw_horizontal_line(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t attr);
void draw_key_value(uint8_t x, uint8_t y, uint16_t len, const char __far* item,
    const char __far* value, uint8_t attr, uint8_t hi_attr, bool selected);

// void tui_itemlist(uint8_t x, uint8_t y, uint8_t w, uint8_t h,
//     uint8_t attr, uint8_t hi_attr, uint8_t selected, uint8_t count,
//     const char* const items[]);