#include "system/system.h"

#include "drivers.h"
#include "debug.h"
#include "string.h"
#include "utility.h"

#include "system/gdt.h"
#include "system/data.h"

#define base_frequency   (3579545 / 3)
#define target_frequency (500)

static bool do_memory_detect(void __far* address)
{
    volatile uint8_t __far* memory = address;

    for (uint8_t pattern = 1; pattern != 0;
        pattern = (uint8_t)(pattern << 1))
    {
        uint8_t a_pattern = pattern;
        uint8_t b_pattern = (uint8_t)~pattern;

        memory[0x0000] = a_pattern;
        memory[0x1000] = b_pattern;

        if (memory[0x0000] != a_pattern ||
            memory[0x1000] != b_pattern)
            return false;
    }

    return true;
}

static uint16_t memory_detect(void)
{
    uint32_t addr = 0L;
    for (; addr < 0xA0000L; addr += 0x2000L)
    {
        void __far* memory = linear_to_fp(addr);

        if (!do_memory_detect(memory))
            break;
    }

    return (uint16_t)(addr >> 10);
}

// extern struct gdt_ptr_t __far* gdt_init(void);

// extern void pmode_enter(gdt_ptr_t __far* gdtptr);
// extern void pmode_leave(void);

typedef void (*function_t)(void);

extern uint16_t pmode_execute(function_t function);

static const gdt_entry_t base_gdt[] =
{
    { 0x0000, 0x0000, 0x00, 0x00, 0x00, 0x00 }, // Null segment
    { 0xFFFF, 0x0000, 0x0F, 0x9A, 0x00, 0x00 }, // 16 bit code segment
    { 0xFFFF, 0x0000, 0x00, 0x92, 0x00, 0x00 }, // 16 bit data segment (data)
    { 0xFFFF, 0x0000, 0x10, 0x92, 0x00, 0x00 }, // 16 bit data segment (memory)
};

static uint16_t memory_do_detect_ext16(void)
{
    volatile gdt_entry_t __far* gdt = (gdt_entry_t __far*)0x00100500L;

    uint32_t memcount = 0;

    for (uint16_t base = 0x10; base < 0x100; ++base)
    {
        gdt[3].base_middle = (uint8_t)base;

        for (uint32_t offset = 0; offset < 0x10000; offset += 0x2000)
        {
            pointer ptr = { .seg = 0x18, .off = (uint16_t)offset };
            
            if (!do_memory_detect(ptr.ptr))
                return (uint16_t)(memcount >> 10);

            memcount += 0x2000;
        }
    }

    return (uint16_t)(memcount >> 10);
}

uint16_t memory_detect_ext16(void)
{
    volatile gdt_entry_t __far* gdt = (gdt_entry_t __far*)0x00000500L;

    gdt[0] = (gdt_entry_t){ 0x0000, 0x0000, 0x00, 0x00, 0x00, 0x00 }; // Null segment
    gdt[1] = (gdt_entry_t){ 0xFFFF, 0x0000, 0x0F, 0x9A, 0x00, 0x00 }; // 16 bit code segment
    gdt[2] = (gdt_entry_t){ 0xFFFF, 0x0000, 0x00, 0x92, 0x00, 0x00 }; // 16 bit data segment (data)
    gdt[3] = (gdt_entry_t){ 0xFFFF, 0x0000, 0x10, 0x92, 0x00, 0x00 }; // 16 bit data segment (memory)

    gdt_ptr_t gdtptr = { sizeof(base_gdt) - 1, (uint32_t)gdt };
    idt_ptr_t idtptr = { 0, 0 };

    asm volatile ("lgdt %0" :: "m"(gdtptr));
    asm volatile ("lidt %0" :: "m"(idtptr));

    return pmode_execute((function_t)memory_do_detect_ext16);
}

static void memory_verify(uint16_t base, uint16_t ext)
{
    uint16_t old_base = as_uint16(cmos_read(0x15), cmos_read(0x14));
    uint16_t old_ext  = as_uint16(cmos_read(0x18), cmos_read(0x17));

    if (old_base != base)
    {
        debug_out("[BIOS] Base memory size mismatch: %d KB != %d KB\n\r",
            old_base, base);

        cmos_write(0x14, (uint8_t)(base >> 0));
        cmos_write(0x15, (uint8_t)(base >> 8));
    }

    if (old_ext != ext)
    {
        debug_out("[BIOS] Extended memory size mismatch: %d KB != %d KB\n\r",
            old_ext, ext);

        cmos_write(0x17, (uint8_t)(ext >> 0));
        cmos_write(0x18, (uint8_t)(ext >> 8));
        cmos_write(0x30, (uint8_t)(ext >> 0));
        cmos_write(0x31, (uint8_t)(ext >> 8));
    }

    // compute_cmos_checksum();
}

void mem_enable_a20(void)
{
    // i8042_clear();
    uint8_t data = 0;

    i8042_send_cmd(0xAA);
    data = i8042_read_data();

    i8042_send_cmd(0xD0);
    data = i8042_read_data();

    debug_out("[BIOS] A20 status: %02X\n\r", data);

    i8042_send_cmd(0xD1);
    i8042_send_data(data | 0x02);

    i8042_send_cmd(0xAE);
}

void mem_detect(void)
{
    uint16_t mem = memory_detect();
    bda->memory_size = mem;

    // Reserve 8 KiB of memory for EBDA
    bda->memory_size -= 0x2000 >> 10;
    bda->ebda_segment = linear_to_seg((uint32_t)bda->memory_size << 10);
    debug_out("[BIOS] EBDA segment: %x\n\r", bda->ebda_segment);

    mem_enable_a20();

    uint16_t ext = memory_detect_ext16();
    
    memory_verify(mem, ext);

    debug_out("[BIOS] Base memory size: %d KB\n\r", mem);
    debug_out("[BIOS] Extended memory size: %d KB\n\r", ext);
}