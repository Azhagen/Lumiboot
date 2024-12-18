#pragma once

#include <stdint.h>
#include "attrib.h"
#include "graphics.h"

void bios_bootstrap(void);

void bios_video_set_mode(uint8_t mode);
void bios_video_set_cursor_type(uint8_t start, uint8_t end);
void bios_video_set_cursor_pos(uint8_t page, uint8_t x, uint8_t y);
void bios_video_get_cursor();
void bios_video_set_page();
void bios_video_write_char_only();
void bios_video_write_char_attr(uint8_t ch, uint8_t page, uint8_t attr, uint16_t count);
void bios_video_write_char_tty(uint8_t ch, uint8_t page, uint8_t attr);
void bios_video_scroll_wnd_up();
void bios_video_scroll_wnd_down(uint8_t lines, uint8_t attr,
    uint8_t x0,  uint8_t x1, uint8_t y0, uint8_t y1);
void bios_video_read_at_cursor(uint8_t page, character_t __far* ch);
void bios_video_get_state();
void bios_video_get_lightpen_pos();
void bios_video_pixel_read();
void bios_video_pixel_write();

uint8_t bios_disk_reset(uint8_t drive);
uint8_t bios_disk_read_sectors(uint8_t drive, uint8_t cylinder,
    uint8_t head, uint8_t sector, uint8_t count, void __far* buffer);

uint16_t bios_keyboard_read_key(void);
uint16_t bios_keyboard_peek_key(void);

uint8_t bios_system_kbd_intercept(uint8_t code);