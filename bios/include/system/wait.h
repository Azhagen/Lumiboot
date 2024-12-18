#pragma once

#include <stdint.h>
#include <stdbool.h>

void pit_wait(uint32_t msecs);
bool pit_wait_until(uint32_t msecs, bool (*function)(void));

void rtc_wait(uint32_t msecs);