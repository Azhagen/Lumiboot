#include "services/serial.h"
#include "drivers.h"

#include "utility.h"
#include "system/data.h"

static const uint16_t baud_rate[] = 
{
    110,  150,  300,  600,
    1200, 2400, 4800, 9600
};

static bool is_receive_ready(uint8_t port);
static bool is_transmit_ready(uint8_t port);

void serial_handler(registers_t __seg_ss* const regs)
{
    switch (regs->ah)
    {
        case 0x00: serial_initialize(regs); break;
        case 0x01: serial_transmit(regs); break;
        case 0x02: serial_receive(regs); break;
        case 0x03: serial_status(regs); break;
    }
}

void serial_initialize(registers_t __seg_ss* const regs)
{
    uint8_t port  = lo(regs->dx);
    uint8_t value = regs->al;

    if (port >= array_size(bda->com_iobase))
        return;

    uint16_t uart = bda->com_iobase[port];

    uint8_t baud_bits = (value >> 5) & 0x7;
    uart_set_baud(uart, baud_rate[baud_bits]);

    uint8_t params = (value >> 0) & 0x3F;
    uart_set_settings(uart, params);

    regs->ah = uart_line_status(uart);
    regs->al = uart_modem_status(uart);
}

void serial_transmit(registers_t __seg_ss* const regs)
{
    uint8_t port  = lo(regs->dx);
    uint8_t value = regs->al;

    if (port >= array_size(bda->com_iobase))
        return;

    uint16_t uart = bda->com_iobase[port];

    if (!is_transmit_ready(port)) {
        regs->ah = uart_line_status(uart) | 0x80;
        return;
    }

    uart_transmit(uart, value);
    regs->ah = uart_line_status(uart);
}

void serial_receive(registers_t __seg_ss* const regs)
{
    uint8_t port = lo(regs->dx);

    if (port >= array_size(bda->com_iobase))
        return;

    uint16_t uart = bda->com_iobase[port];

    if (!is_receive_ready(port)) {
        regs->ah = uart_line_status(uart) | 0x80;
        return;
    }

    regs->al = uart_receive(port);
    regs->ah = uart_line_status(uart);
}

void serial_status(registers_t __seg_ss* const regs)
{
    uint8_t port = lo(regs->dx);

    if (port >= array_size(bda->com_iobase))
        return;

    regs->ah = uart_line_status(port);
    regs->al = uart_modem_status(port);
}

static bool is_receive_ready(uint8_t port)
{
    bda->com_timeout[port] = 0xFF;
    while (!uart_is_receive_ready(port) && bda->com_timeout[port]-- != 0);
    return bda->com_timeout[port] != 0;
}

static bool is_transmit_ready(uint8_t port)
{
    bda->com_timeout[port] = 0xFF;
    while (!uart_is_transmit_ready(port) && bda->com_timeout[port]-- != 0);
    return bda->com_timeout[port] != 0;
}