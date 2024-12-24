#include "debug/gdbstub.h"
#include "system/data.h"

#include "drivers.h"
#include "debug.h"
#include "string.h"

struct __packed dbg_registers
{
    uint16_t di;
    uint16_t si;
    uint16_t bp;
    uint16_t sp;
    uint16_t bx;
    uint16_t dx;
    uint16_t cx;
    uint16_t ax;
    uint16_t ss;
    uint16_t es;
    uint16_t ds;
    uint16_t ip;
    uint16_t cs;
    uint16_t flags;
};

struct __packed gdb_state
{
    union {
    struct {
    uint32_t eax;
    uint32_t ecx;
    uint32_t edx;
    uint32_t ebx;
    uint32_t esp;
    uint32_t ebp;
    uint32_t esi;
    uint32_t edi;
    uint32_t eip;
    uint32_t eflags;
    uint32_t cs;
    uint32_t ss;
    uint32_t ds;
    uint32_t es;
    uint32_t fs;
    uint32_t gs;
    };
    uint32_t regs[16];
    };
};

uint8_t gdb_uart_read(void)
{
    return uart_read(1);
}

uint8_t gdb_uart_write(uint8_t ch)
{
    uart_write(1, ch);
    return 0;
}

uint8_t gdb_get_val(uint8_t ch)
{
    if (ch >= '0' && ch <= '9')
        return (uint8_t)(ch - '0'); // Convert '0'-'9' to 0-9
    else if (ch >= 'A' && ch <= 'F')
        return (uint8_t)(ch - 'A' + 10); // Convert 'A'-'F' to 10-15
    else if (ch >= 'a' && ch <= 'f')
        return (uint8_t)(ch - 'a' + 10); // Convert 'a'-'f' to 10-15
    else
        return 0;
}

uint32_t gdb_decode_hex(uint8_t __far* packet, size_t index, size_t len)
{
    uint32_t value = 0;

    for (size_t i = 0; i < len; i++)
        value = (value << 4) | gdb_get_val(packet[index + i]);

    return value;
}

uint8_t gdb_get_number(uint8_t value)
{
    return "0123456789ABCDEF"[value & 0xF];
}

uint8_t gdb_encode_hex(uint8_t __far* buffer, uint8_t __far* value,
    size_t length, size_t buffer_size)
{
    if (buffer_size < length * 2)
        return GDB_ERROR;

    for (size_t i = 0, j = 0; i < length; i++, j += 2)
    {
        buffer[j + 0] = gdb_get_number(value[i] >> 4);
        buffer[j + 1] = gdb_get_number(value[i] & 0xF);
    }

    buffer[length * 2] = '\0';
    return GDB_SUCCESS;
}

uint8_t gdb_receive_packet(uint8_t __far* packet, size_t __seg_ss* packet_len, size_t buffer_len)
{
    while (true)
    {
        uint8_t ch = gdb_uart_read();

        if (ch == GDB_EOF)
            return GDB_ERROR;
        
        if (ch == '$')
            break;
    }

    size_t len = 0;

    while (true)
    {
        uint8_t ch = gdb_uart_read();

        if (ch == GDB_EOF)
            return GDB_ERROR;

        if (ch == '#')
            break;

        if (len >= buffer_len)
            return GDB_ERROR;

        packet[len++] = ch;
    }

    packet[len] = '\0';
    *packet_len = len;

    debug_out("[BIOS] GDB packet: %s\n\r", packet);

    uint8_t sum[2] = { gdb_uart_read(), gdb_uart_read() };
    uint8_t checksum = 0;

    for (size_t i = 0; i < len; ++i)
        checksum = (uint8_t)(checksum + packet[i]);

    if (checksum != gdb_decode_hex(sum, 0, 2))
        return GDB_EOF;

    gdb_uart_write('+');

    return GDB_SUCCESS;
}

uint8_t gdb_send_packet(const uint8_t __far* packet, uint32_t len)
{
    gdb_uart_write('$');

    uint8_t checksum = 0;

    // if (packet != NULL)
    //     debug_out("-> %s\n\r", packet);

    for (uint32_t i = 0; i < len; ++i)
    {
        uint8_t ch = packet[i];
        checksum   = (uint8_t)(checksum + ch);
        gdb_uart_write(ch);
    }

    gdb_uart_write('#');
    gdb_uart_write(gdb_get_number(checksum >> 4));
    gdb_uart_write(gdb_get_number(checksum & 0xF));

    uint8_t ack = gdb_uart_read();

    if (ack != '+')
        return GDB_ERROR;
    
    return GDB_SUCCESS;
}


uint8_t gdb_read_registers(gdb_state_t __far* state,
    uint8_t __far* buffer, size_t buffer_len)
{
    // if (packet_len != 1)
    //     return gdb_send_packet(NULL, 0);

    if (gdb_encode_hex(buffer, (uint8_t __far*)state->regs,
        array_size(state->regs) * 4, buffer_len) != GDB_SUCCESS)
        return GDB_ERROR;

    return gdb_send_packet(buffer, sizeof(state->regs) * 2);
}

uint8_t gdb_read_register(gdb_state_t __far* state,
    uint8_t __far* buffer, size_t buffer_len)
{
    size_t index = (size_t)gdb_decode_hex(buffer, 1, 2);
    uint32_t value = 0;

    if (index >= GDB_X86_REGISTER_COUNT)
    {
        if (gdb_encode_hex(buffer, (uint8_t __far*)&value, 4, buffer_len) != GDB_SUCCESS)
            return GDB_ERROR;
        return gdb_send_packet(buffer, sizeof(value) * 2);
    }

    value = state->regs[index];
    if (gdb_encode_hex(buffer, (uint8_t __far*)&value, 4, buffer_len) != GDB_SUCCESS)
        return GDB_ERROR;

    return gdb_send_packet(buffer, sizeof(value) * 2);
}

uint8_t gdb_send_signal(uint8_t signal)
{
    uint8_t packet[3] = {};

    packet[0] = 'S';
    packet[1] = gdb_get_number(signal >> 4);
    packet[2] = gdb_get_number(signal & 0xF);

    return gdb_send_packet(packet, 3);
}

size_t gdb_next_index(uint8_t __far* packet, size_t index, size_t packet_len)
{
    while (index < packet_len && packet[index] == ',')
        index++;

    return index;
}

size_t gdb_token_length(uint8_t __far* packet, size_t index, size_t packet_len)
{
    size_t length = 0;

    while (index < packet_len && packet[index] != ',')
    {
        length++;
        index++;
    }

    return length;
}

uint8_t gdb_mem_read(uint32_t addr, uint8_t __seg_ss* value)
{
    pointer ptr = (pointer)linear_to_fp(addr);
    *value = *(uint8_t __far*)(ptr.ptr);
    return GDB_SUCCESS;
}

uint8_t gdb_read_memory(gdb_state_t __far* state,
    uint8_t __far* buffer, size_t packet_len, size_t buffer_len)
{
    size_t index0  = 1;
    size_t length0 = gdb_token_length(buffer, index0, packet_len);
    
    size_t index1  = gdb_next_index(buffer, index0 + length0, packet_len);
    size_t length1 = gdb_token_length(buffer, index1, packet_len);

    // debug_out("index0: %x, length0: %x\n\r", index0, length0);
    // debug_out("index1: %x, length1: %x\n\r", index1, length1);

    uint32_t addr  = gdb_decode_hex(buffer, index0, length0);
    uint32_t count = gdb_decode_hex(buffer, index1, length1);

    // debug_out("[BIOS] Reading memory at %08lX, count %lx\n\r", addr, count);
    uint8_t value = 0;

    for (size_t i = 0; i < count; ++i)
    {
        if (gdb_mem_read(addr + i, &value) != GDB_SUCCESS)
            return GDB_ERROR;

        if (gdb_encode_hex(buffer + i * 2, &value, 1, buffer_len) != GDB_SUCCESS)
            return GDB_ERROR;
    }

    // debug_out("[BIOS] Encoded memory: %s\n\r", buffer);

    return gdb_send_packet(buffer, count * 2);
}

uint8_t gdb_step(gdb_state_t __far* state)
{
    state->eflags |= 0x100; // Set the trap flag
    return GDB_SUCCESS;
}

uint8_t gdb_continue(gdb_state_t __far* state)
{
    state->eflags &= (uint32_t)~0x100; // Clear the trap flag
    return GDB_SUCCESS;
}

void gdb_main(gdb_state_t __far* state, uint8_t signal)
{
    debug_puts("[BIOS] Entering GDB mode\n\r");

    gdb_send_signal(signal);

    while (true)
    {
        ebda_t __far* ebda = get_ebda();
        size_t buffer_len = 0;

        if (gdb_receive_packet(ebda->gdb_buffer, &buffer_len,
            sizeof(ebda->gdb_buffer)) != GDB_SUCCESS)
            continue;

        switch (ebda->gdb_buffer[0])
        {
            case 'g': gdb_read_registers(state, ebda->gdb_buffer, sizeof(ebda->gdb_buffer)); break;
            case 'p': gdb_read_register(state, ebda->gdb_buffer, sizeof(ebda->gdb_buffer)); break;
            case 'm': gdb_read_memory(state, ebda->gdb_buffer, buffer_len, sizeof(ebda->gdb_buffer)); break;

            case 'c': gdb_continue(state); return;
            case 's': gdb_step(state); return;
            case '?': gdb_send_signal(signal); break;

            default:
                // debug_out("[BIOS] Unknown GDB command: %s\n\r", ebda->gdb_srcbuf);
                gdb_send_packet(NULL, 0);
                break;
        }
    }
}

static void gdb_to_state(gdb_state_t __seg_ss* gdb_state,
    dbg_registers_t __seg_ss* state)
{
    gdb_state->eax = state->ax;
    gdb_state->ecx = state->cx;
    gdb_state->edx = state->dx;
    gdb_state->ebx = state->bx;
    gdb_state->esp = state->sp;
    gdb_state->ebp = state->bp;
    gdb_state->esi = state->si;
    gdb_state->edi = state->di;
    gdb_state->eip = state->ip;
    gdb_state->eflags = state->flags;
    gdb_state->cs = state->cs;
    gdb_state->ss = state->ss;
    gdb_state->ds = state->ds;
    gdb_state->es = state->es;
    gdb_state->fs = 0;
    gdb_state->gs = 0;

    gdb_state->eip = (uint32_t)state->cs * 16 + state->ip;
    gdb_state->esp = (uint32_t)state->ss * 16 + state->sp;
}

void gdb_from_state(dbg_registers_t __seg_ss* state,
    gdb_state_t __seg_ss* gdb_state)
{
    state->ax = lo16(gdb_state->eax);
    state->cx = lo16(gdb_state->ecx);
    state->dx = lo16(gdb_state->edx);
    state->bx = lo16(gdb_state->ebx);
    state->bp = lo16(gdb_state->ebp);
    state->si = lo16(gdb_state->esi);
    state->di = lo16(gdb_state->edi);
    state->flags = lo16(gdb_state->eflags);
    state->ds = lo16(gdb_state->ds);
    state->es = lo16(gdb_state->es);
}

void uart_interrupt(dbg_registers_t __seg_ss* regs)
{
    pic_send_eoi(4);

    if (uart_read(1) == 0x03)
    {
        gdb_state_t gdb_state;
        gdb_to_state(&gdb_state, regs);
        gdb_main(&gdb_state, 5);
        gdb_from_state(regs, &gdb_state);
    }
}

void trap_interrupt(dbg_registers_t __seg_ss* regs)
{
    gdb_state_t gdb_state;
    gdb_to_state(&gdb_state, regs);
    gdb_main(&gdb_state, 5);
    gdb_from_state(regs, &gdb_state);
}

void gdb_init(void)
{
    extern void interrupt_dbg(void);
    extern void interrupt_0Bh(void);

    uart_init(1, 115200L, true);

    interrupt_install(0x01, (uint16_t)interrupt_dbg);
    interrupt_install(0x03, (uint16_t)interrupt_dbg);
    interrupt_install(0x0B, (uint16_t)interrupt_0Bh);

    asm volatile ("int $0x03" ::: "memory");
}