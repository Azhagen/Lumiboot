#pragma once

#include <stdint.h>
#include <stdbool.h>

#include "interrupt.h"

void uart_detect(void);

void uart_set_baud(uint16_t port, uint32_t baudrate);
void uart_set_settings(uint16_t port, uint8_t settings);
void uart_transmit(uint16_t port, uint8_t value);
uint8_t uart_receive(uint16_t port);
bool uart_is_transmit_ready(uint16_t port);
bool uart_is_receive_ready(uint16_t port);
uint8_t uart_line_status(uint16_t port);
uint8_t uart_modem_status(uint16_t port);

void uart_init(uint8_t id, uint32_t baudrate, bool irq);
void uart_write(uint8_t id, uint8_t ch);
uint8_t uart_read(uint8_t id);
