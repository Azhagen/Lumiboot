#include "debug.h"
#include "system/data.h"
#include "io.h"
#include "print.h"

#include "drivers.h"
#include "debug/gfx.h"

#include <stddef.h>
#include "stdio.h"

extern int print(char __far* buf, size_t size, const char __far* fmt, va_list __seg_ss* args);

void debug_init(void)
{
#ifdef NDEBUG
    return;
#endif

    // uart_init(0);
    // uart_init(1);
    // debug_puts("\033[2J\033[1;1H");

    switch (bda->debug_device)
    {
        // case DBG_MDA:  debug_gfx_init(); break;
        case DBG_COM1: uart_init(0);
            // debug_puts("\n\r"); break;
            debug_puts("\033[2J\033[1;1H"); break;
        case DBG_COM2: uart_init(1);
            // debug_puts("\n\r"); break;
            debug_puts("\033[2J\033[1;1H"); break;
        case DBG_COM3: uart_init(2);
            debug_puts("\033[2J\033[1;1H"); break;
        case DBG_COM4: uart_init(3);
            debug_puts("\033[2J\033[1;1H"); break;
        case DBG_0xE9: break;

        default: return;
    }
}

void debug_set_output(uint8_t device)
{
#ifdef NDEBUG
    return;
#endif

    bda->debug_device = device;
}

void debug_putc(uint8_t ch)
{
#ifdef NDEBUG
    return;
#endif

    // uart_write(0, ch);
    // uart_write(1, ch);

    switch (bda->debug_device)
    {
        // case DBG_MDA:  debug_gfx_tty_write(ch, 0x7); break;
        case DBG_COM1: uart_write(0, ch); break;
        case DBG_COM2: uart_write(1, ch); break;
        case DBG_COM3: uart_write(2, ch); break;
        case DBG_COM4: uart_write(3, ch); break;
        case DBG_0xE9: io_write(0xE9, ch); break;

        default: return;
    }
}

void debug_puts(const char __far* str)
{
#ifdef NDEBUG
    return;
#endif

    while (*str)
        debug_putc(*str++);
}

void debug_putx(uint32_t value)
{
#ifdef NDEBUG
    return;
#endif

    static const char hex[] = "0123456789ABCDEF";

    debug_putc('0');
    debug_putc('x');

    for (int i = 28; i >= 0; i -= 4)
        debug_putc(hex[(value >> i) & 0xF]);
    
    debug_putc('\n');
    debug_putc('\r');
}

void debug_out(const char __far* fmt, ...)
{
#ifdef NDEBUG
    return;
#endif

    va_list ap;
    va_start(ap, fmt);
    char __far* buffer = (char __far*)get_ebda()->buffer;
    print(buffer, 2048, fmt, &ap);
    debug_puts(buffer);
    va_end(ap);
}