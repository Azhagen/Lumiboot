#include "bios.h"
#include "attrib.h"
#include "io.h"
#include "debug.h"
#include "interrupt.h"
#include "utility.h"

static void int10h_call(bioscall __seg_ss* const regs)
{
    asm volatile (
        "int $0x10\n\r"
        : "+a"(regs->ax)
        : "b"(regs->bx), "c"(regs->cx), "d"(regs->dx)
        : "memory", "cc", "si", "di", "es", "ds", "bp");
}

void bios_video_set_mode(uint8_t mode)
{
    bioscall regs = {0};
    regs.ah = 0x00;
    regs.al = mode;
    int10h_call(&regs);
}

void bios_set_cursor_type(uint8_t start, uint8_t end)
{
    bioscall regs = {0};
    regs.ah = 0x01;
    regs.ch = start;
    regs.cl = end;
    int10h_call(&regs);
}

void bios_move_cursor(uint8_t page, uint8_t x, uint8_t y)
{
    bioscall regs = {0};
    regs.ah = 0x02;
    regs.bh = page;
    regs.dl = x;
    regs.dh = y;
    int10h_call(&regs);
}

void bios_write_char_tty(uint8_t ch, uint8_t page, uint8_t attr)
{
    bioscall regs = {0};
    regs.ah = 0x0E;
    regs.al = ch;
    regs.bh = page;
    regs.bl = attr;
    int10h_call(&regs);
}

void bios_write_char_attr(uint8_t ch, uint8_t page,
    uint8_t attr, uint16_t count)
{
    bioscall regs = {0};
    regs.ah = 0x09;
    regs.al = ch;
    regs.bh = page;
    regs.bl = attr;
    regs.cx = count;
    int10h_call(&regs);
}


void bios_read_at_cursor(uint8_t page, character_t __far* ch)
{
    bioscall regs = {0};
    regs.ah = 0x08;
    regs.bh = page;
    int10h_call(&regs);
    *ch     = *(character_t __far*)&regs.ax;
}

void bios_scroll_wnd_down(uint8_t lines, uint8_t attr,
    uint8_t x0,  uint8_t x1, uint8_t y0, uint8_t y1)
{
    bioscall regs = {0};
    regs.ah = 0x06;
    regs.al = lines;
    regs.bh = attr;
    regs.ch = y0;
    regs.cl = x0;
    regs.dh = y1;
    regs.dl = x1;
    int10h_call(&regs);
}