#include "services/video.h"

#include <stddef.h>
#include <assert.h>
#include <stdbool.h>

#include "utility.h"
#include "system/data.h"
#include "io.h"
#include "debug.h"
#include "interrupt.h"
#include "system/system.h"
#include "string.h"

#include "data/charset.h"
#include "data/video.h"

#include "drivers.h"
#include "string.h"

// TODO: seperate bios code and video code

#define MDA_IOBASE 0x3B4
#define MDA_FBSIZE 0x1000
#define MDA_FBADDR (farptr_t)0xB0000000L

#define CGA_IOBASE 0x3D4
#define CGA_FBSIZE 0x4000
#define CGA_FBADDR (farptr_t)0xB0008000L

#define VIDEO_ENABLE     (1 << 3)

#define MDA_HIRES        (1 << 0)

#define CGA_BLINKING     (1 << 5)
#define CGA_HIRES_GFX    (1 << 4)
// #define CGA_VIDEO_ENABLE (1 << 3)
#define CGA_BLACK_WHITE  (1 << 2)
#define CGA_GFX_MODE     (1 << 1)
#define CGA_HIRES        (1 << 0)

#define CRTC_CURSOR_STA   10
#define CRTC_CURSOR_END   11
#define CRTC_CURSOR_HI    14
#define CRTC_CURSOR_LO    15

#define HEIGHT 25

#define MDA 0
#define CGA 1

#define MODE_40X25_CGA 0
#define MODE_80X25_CGA 1
#define MODE_GFX_CGA   2
#define MODE_80X25_MDA 3

#define EGA_VGA     0
#define COLOR_40x25 1
#define COLOR_80x25 2
#define MONO_80x25  3

#define CGA_GFX_OFF 0x2000


#define BDA_XPOS 0x50   // Cursor position x
#define BDA_YPOS 0x51   // Cursor position y

typedef struct regs video_regs;

static uint8_t get_width(uint8_t mode)
{
    return video_table[12 * 4 + mode];
}

static uint8_t video_get_ctrl(uint8_t mode)
{
    return video_table[12 * 4 + 8 + mode];
}

static uint8_t video_get_moff(uint8_t mode)
{
    return (mode < 2) ? 0 : (mode < 4) ? 1 : (mode < 7) ? 2 : 3;
}

static void set_ctrl(uint16_t iobase, uint8_t value)
{
    io_write(iobase + 4, value);
}

static void set_crtc(uint16_t iobase, uint8_t offset, uint8_t value)
{
    io_write(iobase + 0, offset);
    io_write(iobase + 1, value);
}

static uint8_t get_status(uint16_t iobase)
{
    return io_read(iobase + 6);
}

static bool is_mode_gfx(uint8_t mode)
{
    return mode >= 4 && mode <= 6;
}

static void clear_screen(uint8_t mode)
{
    uint16_t size = (mode != 7) ? CGA_FBSIZE : MDA_FBSIZE;
    farptr_t fbuf = (mode != 7) ? CGA_FBADDR : MDA_FBADDR;
    uint16_t val  = is_mode_gfx(mode) ? 0x00 : 0x0720;

    fmemset_16(fbuf, val, size);
}

static void set_cursor_pos(uint8_t x, uint8_t y)
{
    uint8_t mode  = bda->video_mode;
    uint16_t port = (mode != 7) ? CGA_IOBASE : MDA_IOBASE;
    size_t offset = (size_t)(y * get_width(mode) + x);

    set_crtc(port, CRTC_CURSOR_HI, hi(offset));
    set_crtc(port, CRTC_CURSOR_LO, lo(offset));
}

static void write_char(uint8_t page, uint8_t ch,
    uint8_t attr, uint8_t has_attr)
{
    uint8_t mode = bda->video_mode;

    if (is_mode_gfx(mode))
    {
        // TODO: Implement graphics mode
    }
    else
    {
        position pos = bda->cursor_pos[page];
        uint16_t siz = (mode != 7) ? CGA_FBSIZE : MDA_FBSIZE;
        farptr_t buf = (mode != 7) ? CGA_FBADDR : MDA_FBADDR;
        uint16_t port = (mode != 7) ? CGA_IOBASE : MDA_IOBASE;
        uint16_t off = (uint16_t)(page * HEIGHT * get_width(mode) + pos.y *
           (uint16_t)(get_width(mode) * 2u) + pos.x * 2u);

        while ((get_status(port) & 0x01) == 0);
        volatile uint8_t __far* buffer = buf;

        if (off >= siz) return;

        buffer[off]     = ch;
        buffer[off + 1] = has_attr ? attr : buffer[off + 1];
    }
}

static void write_char_tty(uint8_t page, position __seg_ss* pos,
    uint8_t ch, uint8_t color)
{
    if (is_mode_gfx(bda->video_mode))
        write_char(page, ch, color, true);
    else
        write_char(page, ch, 0x07, true);

    uint8_t width = lo(bda->screen_width);
    if (pos->x++ >= width)
    {
        pos->x = 0;
        pos->y++;
    }
}

static inline farptr_t get_page_address(uint8_t mode,
    uint8_t page, uint8_t stride)
{
    farptr_t fbuf = (mode != 7) ? CGA_FBADDR : MDA_FBADDR;
    return fbuf + (stride * 25) * page;
}

static void move_stride(farptr_t dst, farptr_t src,
    uint16_t len, int16_t stride, int16_t lines)
{
    if (src < dst)
    {
        dst += stride * (lines - 1);
        src += stride * (lines - 1);
        stride = -stride;
    }

    while (lines--)
    {
        fmemcpy_16(dst, src, len);
        dst += stride;
        src += stride;
    }
}

static void video_move_lines_txt(farptr_t ptr, uint8_t stride,
    uint8_t x, uint8_t y, uint8_t width, uint8_t height, int8_t lines)
{
    farptr_t dst = ptr + (y * stride) + (x * 2);
    farptr_t src = ptr + (y * stride) + (x * 2) + (lines * stride);

    move_stride(dst, src, width, stride, height);
}

static void set_stride(farptr_t dst, uint16_t value,
    uint8_t len, int16_t stride, int16_t lines)
{
    while (lines--)
    {
        fmemset_16(dst, value, len);
        dst += stride;
    }
}

static void video_set_lines_txt(farptr_t ptr, uint8_t stride,
    uint8_t x, uint8_t y, uint8_t width, uint8_t height, uint8_t attr)
{
    uint16_t value = (uint16_t)(attr << 8) | ' ';

    farptr_t dst = ptr + (y * stride) + (x * 2);
    set_stride(dst, value, width, stride, height);
}

static void do_scroll_window(uint8_t page, uint8_t x, uint8_t y,
    uint8_t width, uint8_t height, uint8_t attr, int8_t lines)
{
    uint8_t mode   = bda->video_mode;
    uint8_t stride = lo(bda->screen_width * 2);

    farptr_t fb = get_page_address(mode, page, stride);

    if (!lines)
    {
        video_set_lines_txt(fb, stride, x, y,
            width, height, attr);
    }
    else if (lines > 0)
    {
        height = (uint8_t)(height - lines);
        video_move_lines_txt(fb, stride, x, y,
            width, height, lines);

        y      = (uint8_t)(y + height);
        height = (uint8_t)(lines);
        video_set_lines_txt(fb, stride, x, y,
            width, height, attr);
    }
    else
    {
        y      = (uint8_t)(y - lines);
        height = (uint8_t)(height + lines);
        video_move_lines_txt(fb, stride, x, y,
            width, height, lines);

        y      = (uint8_t)(y + lines);
        height = (uint8_t)(-lines);
        video_set_lines_txt(fb, stride, x, y,
            width, height, attr);
    }
}

static void scroll_window(uint8_t count, uint8_t attr,
    uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, int8_t dir)
{
    uint8_t buf_height = HEIGHT;
    uint8_t buf_width  = lo(bda->screen_width);

    if (y1 >= buf_height)
        y1 = (uint8_t)(buf_height - 1);
    if (x1 >= buf_width)
        x1 = (uint8_t)(buf_width - 1);

    uint8_t width  = (uint8_t)(x1 - x0 + 1);
    uint8_t height = (uint8_t)(y1 - y0 + 1);

    if (width == 0 || height == 0)
        return;

    if (count >= height)
        count = 0;

    int8_t lines = (int8_t)(count * dir);

    do_scroll_window(bda->active_video_page, x0, y0, 
        width, height, attr, lines);
}

static void set_cursor_size(uint8_t mode, uint8_t begin, uint8_t end)
{
    uint16_t port = (mode != 7) ? CGA_IOBASE : MDA_IOBASE;

    set_crtc(port, CRTC_CURSOR_STA, begin);
    set_crtc(port, CRTC_CURSOR_END, end);
}

static uint16_t read_char(uint8_t page, uint8_t x, uint8_t y)
{
    uint8_t mode  = bda->video_mode;
    farptr_t fbuf = (mode != 7) ? CGA_FBADDR : MDA_FBADDR;

    uint8_t width = lo(bda->screen_width);
    size_t offset = (size_t)(page * width * HEIGHT);
    size_t value  = (size_t)(y * width + x) + offset;

    uint16_t __far* buffer = fbuf;
    return buffer[value];
}

void video_set_mode(video_regs __seg_ss* const regs)
{
    // debug_out("video_set_mode(%x)\n", regs->al);

    uint8_t mode = regs->al;
    if (mode > 7) return;

    uint16_t port = (mode != 7) ? CGA_IOBASE : MDA_IOBASE;
    uint8_t  ctrl = video_get_ctrl(mode);
    uint8_t  moff = video_get_moff(mode);

    for (uint8_t i = 0; i < 8; ++i)
    {
        bda->cursor_pos[i].x = 0;
        bda->cursor_pos[i].y = 0;
    }

    set_ctrl(port, 0x00);

    for (uint8_t i = 0; i < 12; ++i)
        set_crtc(port, i, video_table[moff * 12 + i]);

    set_ctrl(port, ctrl);
    clear_screen(mode);

    bda->screen_width = get_width(mode);
    bda->video_mode   = mode;

    regs->al = (mode != 6) ? 0x30 : 0x3F;
}

void video_set_cursor_size(video_regs __seg_ss* const regs)
{
    uint8_t begin = regs->ch;
    uint8_t end   = regs->cl;
    uint8_t mode  = bda->video_mode;

    bda->cursor_begin = begin;
    bda->cursor_end   = end;

    set_cursor_size(mode, begin, end);
}

void video_set_cursor_pos(video_regs __seg_ss* const regs)
{
    uint8_t page = regs->bh;
    uint8_t x    = regs->dl;
    uint8_t y    = regs->dh;

    if (page >= 8) return;

    bda->cursor_pos[page].x = x;
    bda->cursor_pos[page].y = y;

    set_cursor_pos(x, y);

    regs->ax = 0;
}

void video_get_cursor_pos(video_regs __seg_ss* const regs)
{
    uint8_t page = regs->bh;
    if (page >= 8) return;

    regs->ch = bda->cursor_begin;
    regs->cl = bda->cursor_end;
    regs->dl = bda->cursor_pos[page].x;
    regs->dh = bda->cursor_pos[page].y;
}

void video_get_light_pen(video_regs __seg_ss* const regs)
{
    (void) regs;
}

void video_set_video_page(video_regs __seg_ss* const regs)
{
    (void) regs;
    // debug_out("video_set_video_page(%d)\n", regs->al);
}

void video_scroll_window_up(video_regs __seg_ss* const regs)
{
    scroll_window(regs->al, regs->bh, regs->cl,
        regs->ch, regs->dl, regs->dh, 1);
}

void video_scroll_window_down(video_regs __seg_ss* const regs)
{
    scroll_window(regs->al, regs->bh, regs->cl,
        regs->ch, regs->dl, regs->dh, -1);
}

void video_read_char_attr(video_regs __seg_ss* const regs)
{
    uint8_t page = regs->bh;
    if (page >= 8) return;

    uint8_t x = bda->cursor_pos[page].x;
    uint8_t y = bda->cursor_pos[page].y;

    regs->ax  = read_char(page, x, y);
}

void video_write_char_attr(video_regs __seg_ss* const regs)
{
    for (; regs->cx != 0; regs->cx--)
        write_char(regs->bh, regs->al, regs->bl, true);
}

void video_write_char_only(video_regs __seg_ss* const regs)
{
    for (; regs->cx != 0; regs->cx--)
        write_char(regs->bh, regs->al, 0x00, false);
}

void video_set_color_palette(video_regs __seg_ss* const regs)
{
    (void) regs;
}

void video_write_pixel(video_regs __seg_ss* const regs)
{
    (void) regs;
}

void video_read_pixel(video_regs __seg_ss* const regs)
{
    (void) regs;
}

void video_write_char_tty(video_regs __seg_ss* const regs)
{
    uint8_t ch    = regs->al;
    uint8_t page  = bda->active_video_page;
    uint8_t color = regs->bl;

    position pos = {
        bda->cursor_pos[page].x,
        bda->cursor_pos[page].y
    };

    switch (ch)
    {
        case 0x7: spk_beep(250);
            break;
        case 0x8: if (pos.x > 0) pos.x--; 
            break;
        case 0xA: pos.y++;
            break;
        case 0xD: pos.x = 0;
            break;

        default: write_char_tty(page, &pos, ch, color);
            break;
    }

    uint8_t width  = lo(bda->screen_width);
    uint8_t height = HEIGHT;

    if (pos.y >= height)
    {
        pos.y--;
        do_scroll_window(page, 0, 0, width, height, 7, 1);
    }

    set_cursor_pos(pos.x, pos.y);

    bda->cursor_pos[page].x = pos.x;
    bda->cursor_pos[page].y = pos.y;
}

void video_get_status(video_regs __seg_ss* const regs)
{
    regs->ah = lo(bda->screen_width);
    regs->al = bda->video_mode;
    regs->bh = bda->active_video_page;
}

void video_handler(video_regs __seg_ss* const regs)
{
    switch (regs->ah)
    {
        case 0x00: video_set_mode(regs);            break;
        case 0x01: video_set_cursor_size(regs);     break;
        case 0x02: video_set_cursor_pos(regs);      break;
        case 0x03: video_get_cursor_pos(regs);      break;
        case 0x04: video_get_light_pen(regs);       break;
        case 0x05: video_set_video_page(regs);      break;
        case 0x06: video_scroll_window_up(regs);    break;
        case 0x07: video_scroll_window_down(regs);  break;
        case 0x08: video_read_char_attr(regs);      break;
        case 0x09: video_write_char_attr(regs);     break;
        case 0x0A: video_write_char_only(regs);     break;
        case 0x0B: video_set_color_palette(regs);   break;
        case 0x0C: video_write_pixel(regs);         break;
        case 0x0D: video_read_pixel(regs);          break;
        case 0x0E: video_write_char_tty(regs);      break;
        case 0x0F: video_get_status(regs);          break;
        default: break;
    }
}