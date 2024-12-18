#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>
#include "attrib.h"

int printf(const char __far* fmt, ...);
int snprintf(char __far* buf, size_t size, const char __far* fmt, ...);

int vprintf(const char __far* fmt, va_list args);
int vsnprintf(char __far* buf, size_t size, const char __far* fmt, va_list args);