#include "drivers/misc/uart.h"

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "io.h"
#include "utility.h"
#include "attrib.h"
#include "system/data.h"
#include "debug.h"

#define SERIAL_COUNT 8

static const uint16_t serial_ports[SERIAL_COUNT] = 
{
    0x3f8, 0x2f8, 0x3e8, 0x2e8,
    0x5f8, 0x4f8, 0x5e8, 0x4e8
};

#define PARITY_NONE (0 << 3)
#define PARITY_EVEN (1 << 3)
#define PARITY_ODD  (3 << 3)
#define PARITY_MASK (7 << 3)
#define STOP_MASK   (1 << 2)
#define WORD_MASK   (3 << 0)

#define SETTINGS_MASK 0x3F

// static const uint8_t parity[] =
// {
//     PARITY_NONE, PARITY_ODD,
//     PARITY_NONE, PARITY_EVEN
// };




#define UART_DIVISOR_LO 0x0
#define UART_DIVISOR_HI 0x1
#define UART_LINE_CTRL  0x3

#define UART_LINE_CTRL 0x3

#define UART_IER 1  // Interrupt Enable Register
#define UART_IIR 2  // Interrupt Identification Register
#define UART_FCR 2  // FIFO Control Register
#define UART_LCR 3  // Line Control Register
#define UART_MCR 4  // Modem Control Register
#define UART_LSR 5  // Line Status Register
#define UART_MSR 6  // Modem Status Register
#define UART_SCR 7  // Scratch Register

#define UART_BRD_LO 0 // Baud Rate Divisor Lo
#define UART_BRD_HI 1 // Baud Rate Divisor Hi

#define UART_LEN_5       (0 << 0)
#define UART_LEN_6       (1 << 0)
#define UART_LEN_7       (2 << 0)
#define UART_LEN_8       (3 << 0)
#define UART_STOP_1      (0 << 2)
#define UART_STOP_2      (1 << 2)
#define UART_PARITY      (1 << 3)
#define UART_PARITY_NONE (0 << 3)
#define UART_PARITY_EVEN (1 << 4)
#define UART_PARITY_ODD  (1 << 4)
#define UART_ENABLE_BRD  (1 << 7)

#define UART_ENABLE_DTR (1 << 0)
#define UART_ENABLE_RTS (1 << 1)
#define UART_LOOPBACK   (1 << 4)

static bool uart_detect_presence(uint16_t port);

void uart_set_baud(uint16_t port, uint32_t baudrate)
{
    uint16_t divisor = (uint16_t)(115200UL / baudrate);

    // uint8_t val = io_read(port + UART_LCR);

    io_write(port + UART_LCR, 0x80);
    io_write(port + UART_BRD_LO, lo(divisor));
    io_write(port + UART_BRD_HI, hi(divisor));
    io_write(port + UART_LCR, 0x00);
}

void uart_set_settings(uint16_t port, uint8_t settings)
{
    uint8_t value = io_read(port + UART_LCR);
    value = (uint8_t)((value & ~SETTINGS_MASK) | settings);

    io_write(port + UART_LCR, value);
}

uint8_t uart_line_status(uint16_t port)
{
    return io_read(port + UART_LSR);
}

uint8_t uart_modem_status(uint16_t port)
{
    return io_read(port + UART_MSR);
}

bool uart_is_transmit_ready(uint16_t port)
{
    return (io_read(port + UART_LSR) & 0x20) != 0;
}

bool uart_is_receive_ready(uint16_t port)
{
    return (io_read(port + UART_LSR) & 0x01) != 0;
}

void uart_transmit(uint16_t port, uint8_t value)
{
    io_write(port, value);
}

uint8_t uart_receive(uint16_t port)
{
    return io_read(port);
}

void uart_detect(void)
{
    uint8_t offset = 0;

    for (uint8_t i = 0; i < SERIAL_COUNT; ++i)
    {
        if (!uart_detect_presence(serial_ports[i]))
            continue;

        bda->com_iobase[offset++] = serial_ports[i];

        if (offset > 3)
            return;
    }
}

static inline void uart_disable_irqs(uint16_t port)
{
    io_write(port + UART_IER, 0x00);
}

static inline void uart_enable_irqs(uint16_t port)
{
    io_write(port + UART_IER, 0x01);
    debug_out("[UART] IRQs enabled, value = %x\n\r", io_read(port + UART_IER));
}

static inline void uart_set_line(uint16_t port, uint8_t value)
{
    io_write(port + UART_LCR, value);
}

static inline void uart_set_fifo(uint16_t port, uint8_t value)
{
    io_write(port + UART_FCR, value);
}

static inline void uart_set_modem(uint16_t port, uint8_t value)
{
    io_write(port + UART_MCR, value);
}

static bool is_receive_ready(uint16_t port)
{
    int16_t timeout = 0x7FFF;
    while (!uart_is_receive_ready(port) || --timeout > 0);
    return timeout != 0;
}

static bool is_transmit_ready(uint16_t port)
{
    int16_t timeout = 0x7FFF;
    while (!uart_is_transmit_ready(port) && --timeout > 0);
    return timeout != 0;
    // io_write(0x80, 0xAA);
    // while (!uart_is_transmit_ready(port));
    // return true;
}

static uint8_t uart_read_irr(uint16_t port)
{
    return io_read(port + UART_IIR);
}

static bool uart_detect_presence(uint16_t port)
{
    uart_set_fifo(port, 0x00);
    uint8_t irr = uart_read_irr(port);
    return !(irr & 0xF0);
}

void uart_init(uint8_t id)
{
    if (id >= SERIAL_COUNT)
        return;

    uint16_t port = serial_ports[id];
    // uint16_t port = uart_get_port(id);

    uart_disable_irqs(port);
    // uart_set_baud(port, 9600);
    uart_set_baud(port, 115200L);
    uart_set_line(port, 0x03);
    uart_set_fifo(port, 0xC7);
    uart_set_modem(port, 0x0F);
    // uart_enable_irqs(port);
}

void uart_write(uint8_t id, uint8_t ch)
{
    if (id >= SERIAL_COUNT)
        return;

    uint16_t port = serial_ports[id];

    if (is_transmit_ready(port))
        uart_transmit(port, ch);
}

uint8_t uart_read(uint8_t id)
{
    if (id >= SERIAL_COUNT)
        return 0;

    uint16_t port = serial_ports[id];

    if (is_receive_ready(port))
        return uart_receive(port);

    return 0;
}