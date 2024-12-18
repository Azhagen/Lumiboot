#pragma once

#include <stdarg.h>
#include <stdint.h>
#include <stddef.h>
#include "attrib.h"

void printd(const char __far* fmt, ...);
void vprintd(const char __far* fmt, va_list __far* pvlist);

int snprintf(char __far* str, size_t size, const char __far* fmt, ...);
int vsnprintf(char __far* str, size_t size, const char __far* fmt, va_list vlist);