#include "stdio.h"
#include "system/data.h"

#include "debug.h"

extern int print(char __far* buf, size_t size, const char __far* fmt, va_list __seg_ss* args);

int printf(const char __far* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    char __far* buffer = (char __far*)get_ebda()->buffer;
    int ret = print(buffer, 2048, fmt, &args);
    va_end(args);
    return ret;
}

int snprintf(char __far* buf, size_t size, const char __far* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    int ret = print(buf, size, fmt, &args);
    va_end(args);
    return ret;
}

int vprintf(const char __far* fmt, va_list args)
{
    char __far* buffer = (char __far*)get_ebda()->buffer;
    return print(buffer, 2048, fmt, &args);
}

int vsnprintf(char __far* buf, size_t size, const char __far* fmt, va_list args)
{
    return print(buf, size, fmt, &args);
}