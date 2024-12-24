#include "tui.h"
#include "bios.h"
#include "attrib.h"
#include "debug.h"
#include "system/data.h"
#include "string.h"
#include <stdarg.h>

static void move_cursor(uint8_t x, uint8_t y)
{
    bios_move_cursor(0, x, y);
}

static void write_char(uint8_t ch, uint8_t attr)
{
    bios_write_char_attr(ch, 0, attr, 1);
}

void draw_char_at(uint8_t ch, uint8_t x, uint8_t y, uint8_t attr)
{
    move_cursor(x, y);
    write_char(ch, attr);
}

void draw_text(uint8_t x, uint8_t y, const char __far* str, uint8_t attr)
{
    uint8_t xpos = x;
    uint8_t ypos = y;

    while (*str != 0) {
        if (*str == '\n') {
            xpos = x;
            ypos++;
        } else {
            draw_char_at(*str, xpos, ypos, attr);
            xpos++;
        }
        str++;
    }
    move_cursor(0xFF, 0xFF);
}

void tui_clear(uint8_t x, uint8_t y, uint8_t width, uint8_t height) {
    for (uint8_t ypos = 0; ypos < height; ++ypos) {
        move_cursor(x, y + ypos);
        for (uint8_t xpos = 0; xpos < width; ++xpos) {
            bios_write_char_tty(' ', 0, 0x07);
        }
    }
    move_cursor(0xFF, 0xFF);
}

void tui_image(uint8_t x, uint8_t y, const logo __far* img) {
    for (uint8_t ypos = 0; ypos < img->height; ++ypos) {
        for (uint8_t xpos = 0; xpos < img->width; ++xpos) {
            move_cursor(x + xpos, y + ypos);
            write_char(img->data[ypos * img->width + xpos], 0x07);
        }
    }
    move_cursor(0xFF, 0xFF);
}

void draw_tabs(uint8_t x, uint8_t y, uint8_t w, uint8_t count,
    uint8_t selected, uint8_t attr, uint8_t hi_attr, const char* const items[])
{
    uint8_t xpos = x;
    uint8_t ypos = y;
    size_t total_len = 0;

    for (uint8_t i = 0; i < count; ++i)
        total_len += strlen(items[i]);

    uint8_t space = (uint8_t)((w - total_len) / (count + 1));

    for (uint8_t i = 0; i < w; ++i)
        draw_char_at(' ', x + i, y, attr);

    for (uint8_t i = 0; i < count; ++i)
    {
        uint8_t len = (uint8_t)strlen(items[i]);
        uint8_t item_attr = i == selected ? hi_attr : attr;

        xpos += space;
        if (i == selected) {
            draw_char_at(' ', xpos - 1, y, item_attr);
            draw_text(xpos, ypos, items[i], item_attr);
            draw_char_at(' ', xpos + len, y, item_attr);
        } else {
            draw_text(xpos, ypos, items[i], item_attr);
        }
        xpos += len;
    }
}

void draw_border(uint8_t x, uint8_t y, uint8_t w, uint8_t h,
    uint8_t attr, uint8_t border)
{
    uint8_t xpos = x;
    uint8_t ypos = y;

    if (border == BORDER_OMIT) return;

    uint8_t top_left, top_right, bottom_left, bottom_right, horizontal, vertical;
    
    if (border == BORDER_LARGE)
    {
        top_left = 0xDA; top_right = 0xBF; bottom_left = 0xC0; bottom_right = 0xD9;
        horizontal = 0xC4; vertical = 0xB3;
    }
    else if (border == BORDER_SIMPLE)
    {
        top_left = 0xC9; top_right = 0xBB; bottom_left = 0xC8; bottom_right = 0xBC;
        horizontal = 0xCD; vertical = 0xBA;
    } 
    else return;

    draw_char_at(top_left, xpos, ypos, attr);
    draw_char_at(top_right, xpos + w - 1, ypos, attr);
    draw_char_at(bottom_left, xpos, ypos + h - 1, attr);
    draw_char_at(bottom_right, xpos + w - 1, ypos + h - 1, attr);

    for (uint8_t i = 1; i < w - 1; ++i) {
        draw_char_at(horizontal, xpos + i, ypos, attr);
        draw_char_at(horizontal, xpos + i, ypos + h - 1, attr);
    }
    for (uint8_t i = 1; i < h - 1; ++i) {
        draw_char_at(vertical, xpos, ypos + i, attr);
        draw_char_at(vertical, xpos + w - 1, ypos + i, attr);
    }
}

void fill_region(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t ch, uint8_t fg, uint8_t bg)
{
    bios_scroll_wnd_down(h, bg << 4 | fg, x, x + w - 1, y, y + h - 1);
}

void tui_menulist(uint8_t x, uint8_t y, uint8_t w, uint8_t h,
    uint8_t attr, uint8_t hi_attr, uint8_t selected,
    uint8_t count, const char* const items[])
{
    uint8_t xpos = x;
    uint8_t ypos = y;

    for (uint8_t i = 0; i < count; ++i)
    {
        draw_text(xpos, ypos, items[i], (i == selected) ? (hi_attr) : (attr));
        ypos++;
    }
}

void draw_horizontal_line(uint8_t x0, uint8_t y0,
    uint8_t x1, uint8_t attr)
{
    for (uint8_t i = x0; i < x1; ++i)
        draw_char_at(0xC4, i, y0, attr);
}

void draw_key_value(uint8_t x, uint8_t y, uint16_t offset,
    const char __far* key, const char __far* value,
    uint8_t attr, uint8_t hi_attr, bool selected)
{
    uint8_t xpos = x;
    uint8_t ypos = y;

    draw_text(xpos, ypos, key, attr);
    xpos += offset;

    if (selected)
        draw_text(xpos, ypos, value, hi_attr);
    else
        draw_text(xpos, ypos, value, attr);
}