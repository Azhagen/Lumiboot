#pragma once

#include <stdint.h>

#define PPI_PORT_A 0
#define PPI_PORT_B 1
#define PPI_PORT_C 2

uint8_t ppi_xt_read(uint8_t port);
void ppi_xt_write(uint8_t port, uint8_t val);

void ppi_xt_init(void);
uint8_t ppi_xt_read_kbd(void);
void ppi_xt_reset_kbd(void);