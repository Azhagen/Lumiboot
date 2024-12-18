#pragma once

#include <stdint.h>

#define I8042_SUCCESS 0
#define I8042_ERROR   1

void i8042_init(void);
void i8042_init_kbd(void); 

uint8_t i8042_clear(void);

uint8_t i8042_send_cmd(uint8_t cmd);
uint8_t i8042_send_data(uint8_t data);
uint8_t i8042_read_data(void);

uint8_t i8042_read_kbd(void);
void i8042_write_kbd(uint8_t data);