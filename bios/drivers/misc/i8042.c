#include "drivers/misc/i8042.h"

#include <stdbool.h>
#include <stdint.h>

#include "io.h"
#include "debug.h"
#include "system/system.h"

#define I8042_DATA 0x60
#define I8042_CTRL 0x64



#define I8042_TIMEOUT 0x00FF

static bool i8042_is_output_full(void)
{
    bool res = (io_read(I8042_CTRL) & (1 << 0)) != 0x00;
    // debug_out("i8042: Output full: %d\r\n", res);
    // debug_out("i8042: CTRL = %02X\r\n", io_read(I8042_CTRL));
    return res;
}

static bool i8042_is_input_full(void)
{
    bool res = (io_read(I8042_CTRL) & (1 << 1)) != 0x00;
    // debug_out("i8042: Input full: %d\r\n", res);
    // debug_out("i8042: CTRL = %02X\r\n", io_read(I8042_CTRL));
    return res;
}

uint8_t i8042_send_cmd(uint8_t cmd)
{
    uint16_t timeout = I8042_TIMEOUT;
    while (i8042_is_input_full() && timeout--);
    if (!timeout) return I8042_ERROR;
    io_write(I8042_CTRL, cmd);
    return I8042_SUCCESS;
}

uint8_t i8042_send_data(uint8_t data)
{
    uint16_t timeout = I8042_TIMEOUT;
    while (i8042_is_input_full() && timeout--);
    if (!timeout) return I8042_ERROR;
    io_write(I8042_DATA, data);
    return I8042_SUCCESS;
}

uint8_t i8042_read_data(void)
{
    while (!i8042_is_output_full());
    return io_read(I8042_DATA);
}

uint8_t i8042_clear(void)
{
    while (i8042_is_output_full())
        io_read(I8042_DATA);
    return I8042_SUCCESS;
}

static uint8_t i8042_disable_ch0(void)
{
    return i8042_send_cmd(0xAD);
}

static void i8042_enable_ch0(void)
{
    i8042_send_cmd(0xAE);
}

static uint8_t i8042_disable_ch1(void)
{
    return i8042_send_cmd(0xA7);
}

static uint8_t i8042_read_config(void)
{
    i8042_send_cmd(0x20);
    return i8042_read_data();
}

static uint8_t i8042_write_config(uint8_t byte)
{
    if (i8042_send_cmd(0x60) != I8042_SUCCESS)
        return I8042_ERROR;
    return i8042_send_data(byte);
}

static bool i8042_self_test(void)
{
    i8042_send_cmd(0xAA);
    return i8042_read_data() == 0x55;
}

static bool i8042_test_ch0(void)
{
    i8042_send_cmd(0xAB);
    return i8042_read_data() == 0x00;
}

static uint8_t i8042_read_ch0(void)
{
    while (!i8042_is_output_full());
    return i8042_read_data();
}

static uint8_t i8042_write_ch0(uint8_t data)
{
    uint16_t timeout = I8042_TIMEOUT;
    while (!i8042_is_input_full() && timeout--);
    if (!timeout) return I8042_ERROR;
    return i8042_send_data(data);
}

// static uint8_t i8042_read_ch1(void)
// {
//     i8042_send_cmd(0xC1);
//     return i8042_read_data();
// }

void i8042_init(void)
{
    if (i8042_disable_ch0() != I8042_SUCCESS)
        goto err;
    if (i8042_disable_ch1() != I8042_SUCCESS)
        goto err;

    i8042_clear();

    i8042_write_config(0x00);

    if (!i8042_self_test())
        return debug_puts("i8042: Self test failed\r\n");

    if (!i8042_test_ch0())
        return debug_puts("i8042: Channel 0 test failed\r\n");
    
    i8042_enable_ch0();

    i8042_write_config(0x51);
    return;

err:
    debug_puts("i8042: Failed to initialize\r\n");
}

void i8042_init_kbd(void)
{
    if (i8042_write_ch0(0xFF) != I8042_SUCCESS)
        debug_puts("i8042: KBD reset failed\r\n");

    if (i8042_read_ch0() != 0xFA && i8042_read_ch0() != 0xAA)
        debug_puts("i8042: KBD self test failed\r\n");
}

uint8_t i8042_read_kbd(void)
{
    return i8042_read_data();
}

void i8042_write_kbd(uint8_t data)
{
    i8042_write_ch0(data);
}