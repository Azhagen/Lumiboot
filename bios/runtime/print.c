#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "attrib.h"
#include "utility.h"

#define PRINT_STATE_NORMAL 0
#define PRINT_STATE_LENGTH 1
#define PRINT_STATE_SPECIF 2
#define PRINT_STATE_FORMAT 3

#define LENGTH_STATE_LEN_NONE  0
#define LENGTH_STATE_LEN_BYTE  1
#define LENGTH_STATE_LEN_SHORT 2
#define LENGTH_STATE_LEN_LONG  3
#define LENGTH_STATE_LEN_LLONG 4
#define LENGTH_STATE_LEN_PMAX  5
#define LENGTH_STATE_LEN_SMAX  6
#define LENGTH_STATE_LEN_PDIFF 7
#define LENGTH_STATE_LEN_LDBL  8

typedef struct {
    uint8_t print;
    uint8_t length;
    size_t off;
    va_list __seg_ss* pvlist;
    size_t size;
    size_t pos;
    bool leading_zeros;
    size_t width;
    char __far* buffer;
    const char __far* format;
} state_machine_t;

static void handle_state_normal(state_machine_t __seg_ss* sm);
static void handle_state_length(state_machine_t __seg_ss* sm);
static void handle_state_specif(state_machine_t __seg_ss* sm);
static void handle_state_format(state_machine_t __seg_ss* sm);
static void handle_chr(state_machine_t __seg_ss* sm);
static void handle_str(state_machine_t __seg_ss* sm);
static void handle_sint(state_machine_t __seg_ss* sm);
static void handle_uint(state_machine_t __seg_ss* sm);
static void handle_fptr(state_machine_t __seg_ss* sm);
static void write_chr(state_machine_t __seg_ss* sm, char ch);
static void write_str(state_machine_t __seg_ss* sm, const char __far* str);
static void utoa(state_machine_t __seg_ss* sm, uint32_t value, uint8_t base);
static void itoa(state_machine_t __seg_ss* sm, int32_t val, uint8_t base, bool relative);
static void to_chars(state_machine_t __seg_ss* sm, uint32_t value, uint8_t base, bool negative);
static int32_t read_int(va_list __seg_ss *pvlist, uint8_t length);
static uint32_t read_uint(va_list __seg_ss *pvlist, uint8_t length);
static size_t digits(uint32_t num, uint8_t base);

int print(char __far* buf, size_t size, const char __far* fmt, va_list __seg_ss* args)
{
    state_machine_t sm = {0};
    
    sm.format = fmt;
    sm.buffer = buf;
    sm.size = size;
    sm.pos = 0;
    sm.pvlist = args;
    sm.print = PRINT_STATE_NORMAL;
    sm.length = LENGTH_STATE_LEN_NONE;
    sm.off = 0;
    sm.leading_zeros = false;
    sm.width = 0;

    while (fmt[sm.off] != 0) {
        switch (sm.print) {
            case PRINT_STATE_NORMAL:
                handle_state_normal(&sm);
                break;
            case PRINT_STATE_LENGTH:
                handle_state_length(&sm);
                break;
            case PRINT_STATE_SPECIF:
                handle_state_specif(&sm);
                break;
            case PRINT_STATE_FORMAT:
                handle_state_format(&sm);
                break;
            default:
                break;
        }
    }

    return (int)sm.pos;
}

void handle_state_normal(state_machine_t __seg_ss* sm)
{
    char ch = sm->format[sm->off++];

    if (ch != '%') {
        write_chr(sm, ch);
    } else {
        sm->print = PRINT_STATE_LENGTH;
        sm->leading_zeros = false;
        sm->width = 0;
    }
}

void handle_state_length(state_machine_t __seg_ss* sm)
{
    int ch = sm->format[sm->off];

    if (ch == '0') {
        sm->leading_zeros = true;
        sm->off++;
        ch = sm->format[sm->off];
    }

    while (ch >= '0' && ch <= '9') {
        sm->width = sm->width * 10 + (ch - '0');
        sm->off++;
        ch = sm->format[sm->off];
    }

    switch (ch) {
        case 'h':
            sm->length = LENGTH_STATE_LEN_SHORT;
            break;
        case 'l':
            sm->length = LENGTH_STATE_LEN_LONG;
            break;
        case 'j':
            sm->length = LENGTH_STATE_LEN_PMAX;
            break;
        case 'z':
            sm->length = LENGTH_STATE_LEN_SMAX;
            break;
        case 'L':
            sm->length = LENGTH_STATE_LEN_LDBL;
            break;
        default:
            sm->print = PRINT_STATE_FORMAT;
            return;
    }

    sm->print = PRINT_STATE_SPECIF;
    sm->off++;
}

void handle_state_specif(state_machine_t __seg_ss* sm)
{
    char ch = sm->format[sm->off];
    sm->print = PRINT_STATE_FORMAT;

    if (ch == 'h') {
        sm->length = LENGTH_STATE_LEN_BYTE;
    } else if (ch == 'l') {
        sm->length = LENGTH_STATE_LEN_LLONG;
    } else {
        return;
    }

    sm->off++;
}

void handle_state_format(state_machine_t __seg_ss* sm)
{
    uint8_t ch = sm->format[sm->off];

    switch (ch) {
        case 'c': handle_chr(sm); break;
        case 's': handle_str(sm); break;
        case 'd':
        case 'i': handle_sint(sm); break;
        case 'P': handle_fptr(sm); break;
        case 'o':
        case 'x':
        case 'X':
        case 'u':
        case 'p': handle_uint(sm); break;
        default: break; // FIXME: something broke
    }
    
    sm->print  = PRINT_STATE_NORMAL;
    sm->length = LENGTH_STATE_LEN_NONE;
    sm->off++;
}

void handle_fptr(state_machine_t __seg_ss* sm)
{
    uint8_t radix  = 16;
    pointer farptr = (pointer)(va_arg(*sm->pvlist, uint32_t));
    uint32_t ptr   = ((uint32_t)(farptr.seg) << 4) + farptr.off;

    write_str(sm, "0x");
    utoa(sm, ptr, radix);
}

static inline uint8_t set_radix(uint8_t ch)
{
    switch (ch) {
        case 'o': return 8;
        case 'p':
        case 'x':
        case 'X': return 16;
        default: return 10;
    }
}

static inline uint32_t read_uint(va_list __seg_ss *pvlist, uint8_t length)
{
    switch (length) {
        case LENGTH_STATE_LEN_NONE:
        case LENGTH_STATE_LEN_SHORT:
        case LENGTH_STATE_LEN_BYTE:
        case LENGTH_STATE_LEN_SMAX:
        case LENGTH_STATE_LEN_PDIFF:
            return va_arg(*pvlist, unsigned int);
        case LENGTH_STATE_LEN_LONG:
            return va_arg(*pvlist, unsigned long);
        default:
            return 0; // FIXME: something went wrong
    }
}

void handle_uint(state_machine_t __seg_ss* sm)
{
    uint8_t ch     = sm->format[sm->off];
    uint8_t radix  = set_radix(ch);
    uint32_t value = 0;

    if (ch == 'p') {
        write_str(sm, "0x");
    }

    value = read_uint(sm->pvlist, sm->length);
    utoa(sm, value, radix);
}

void handle_sint(state_machine_t __seg_ss* sm)
{
    int32_t value = read_int(sm->pvlist, sm->length);
    itoa(sm, value, 10, true);
}

void handle_chr(state_machine_t __seg_ss* sm)
{
    write_chr(sm, (char)va_arg(*sm->pvlist, int));
}

void handle_str(state_machine_t __seg_ss* sm)
{
    write_str(sm, va_arg(*sm->pvlist, char __far *));
}

void itoa(state_machine_t __seg_ss* sm, int32_t val, uint8_t base, bool relative)
{
    uint32_t value = (uint32_t)(val);

    bool negative = (val < 0) && relative;
    if (negative && base == 10) {
        value = (uint32_t)(-val);
    }

    to_chars(sm, value, base, negative);
}

void utoa(state_machine_t __seg_ss* sm, uint32_t value, uint8_t base)
{
    to_chars(sm, value, base, false);
}

size_t digits(uint32_t num, uint8_t base)
{
    size_t count = 0;
    do {
        num /= base;
        count++;
    } while (num > 0);
    return count;
}

void to_chars(state_machine_t __seg_ss* sm, uint32_t value, uint8_t base, bool negative)
{
    static const char numbers[] = "0123456789ABCDEF";
    char buffer[33] = {0};

    size_t count = digits(value, base);
    size_t pos   = count - 1 + negative;

    while (value >= base) {
        buffer[pos--] = numbers[value % base];
        value /= base;
    }

    buffer[pos] = numbers[value];

    if (negative && base == 10) {
        buffer[0] = '-';
    }

    // Handle leading zeros
    if (sm->leading_zeros && sm->width > count) {
        size_t padding = sm->width - count;
        for (size_t i = 0; i < padding; i++) {
            write_chr(sm, '0');
        }
    }

    write_str(sm, buffer);
}

void write_chr(state_machine_t __seg_ss* sm, char ch)
{
    if (sm->pos < sm->size - 1) {
        sm->buffer[sm->pos++] = ch;
        sm->buffer[sm->pos] = '\0';
    }
}

void write_str(state_machine_t __seg_ss* sm, const char __far* str)
{
    while (*str && sm->pos < sm->size - 1)
        sm->buffer[sm->pos++] = *str++;

    sm->buffer[sm->pos] = '\0';
}

int32_t read_int(va_list __seg_ss *pvlist, uint8_t length)
{
    switch (length) {
        case LENGTH_STATE_LEN_NONE:
        case LENGTH_STATE_LEN_BYTE:
        case LENGTH_STATE_LEN_SHORT:
        case LENGTH_STATE_LEN_SMAX:
        case LENGTH_STATE_LEN_PDIFF:
            return va_arg(*pvlist, signed int);
        case LENGTH_STATE_LEN_LONG:
            return va_arg(*pvlist, int32_t);
        default:
            return 0; // FIXME: something went wrong
    }
}
