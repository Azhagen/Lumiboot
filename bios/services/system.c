#include "services/system.h"
#include "drivers.h"

#include "utility.h"
#include "debug.h"

#include "data/config.h"
#include "data/system.h"

#include "system/data.h"
#include "system/system.h"
#include "system/gdt.h"
#include "system/pmode.h"

void system_handler(bioscall __seg_ss* const regs)
{
    switch (regs->ah)
    {
        case 0x00: return;
        case 0x01: return;
        case 0x02: return;
        case 0x03: return;
        case 0x4F: system_kbd_intercept(regs);  break;
        case 0x80: system_device_open(regs);    break;
        case 0x81: system_device_close(regs);   break;
        case 0x82: system_program_end(regs);    break;
        case 0x83: system_async_wait(regs);     break;
        case 0x84: system_joy_handler(regs);    break;
        case 0x85: system_request(regs);        break;
        case 0x86: system_sync_wait(regs);      break;
        case 0x87: system_move_blocks(regs);    break;
        case 0x88: system_memory_size(regs);    break;
        case 0x89: system_enter_pmode(regs);    break;
        case 0x8A: system_device_busy(regs);    break;
        case 0x91: system_interrupt_end(regs);  break;
        case 0xC0: system_read_config(regs);    break;

        default: regs->ah = 0; regs->CF = 1;
            break;
    }
}

void system_kbd_intercept(bioscall __seg_ss* const regs)
{
    regs->CF = 0;
}

void system_device_open(bioscall __seg_ss* const regs)
{
    regs->ah = 0;
    regs->CF = 0;
}

void system_device_close(bioscall __seg_ss* const regs)
{
    regs->ah = 0;
    regs->CF = 0;
}

void system_program_end(bioscall __seg_ss* const regs)
{
    regs->ah = 0;
    regs->CF = 0;
}

void system_async_wait(bioscall __seg_ss* const regs)
{
    switch (regs->al)
    {
        case 0x00: sub_wait_interval_set(regs);    break;
        case 0x01: sub_wait_interval_cancel(regs); break;

        default: regs->al--; regs->CF = 1;
            break;
    }
}

void sub_wait_interval_set(bioscall __seg_ss* const regs)
{
    (void) regs;
    // TODO: implement
}

void sub_wait_interval_cancel(bioscall __seg_ss* const regs)
{
    (void) regs;
    // TODO: implement
}

void system_joy_handler(bioscall __seg_ss* const regs)
{
    switch (regs->dx)
    {
        case 0x00: sub_joystick_read_settings(regs); break;
        case 0x01: sub_joystick_read_inputs(regs);   break;

        default: regs->CF = 1;
            break;
    }
}

void sub_joystick_read_settings(bioscall __seg_ss* const regs)
{
    (void) regs;
    // TODO: implement
}

void sub_joystick_read_inputs(bioscall __seg_ss* const regs)
{
    (void) regs;
    // TODO: implement
}

void system_request(bioscall __seg_ss* const regs)
{
    regs->ah = 0;
    regs->CF = 0;
}

void system_sync_wait(bioscall __seg_ss* const regs)
{
    uint32_t msecs = as_uint32(regs->cx, regs->dx);
    if (!msecs) return;

    // TODO: implement
    regs->CF = 0x01;
}

void system_move_blocks(bioscall __seg_ss* const regs)
{
    gdt_entry_t __far* gdt = segoff_to_fp(regs->es, regs->si);

    gdt[0] = (gdt_entry_t){ 0x0000, 0x0000, 0x00, 0x00, 0x00, 0x00 }; // Null segment
    gdt[1] = (gdt_entry_t){ 0xFFFF, 0x0000, 0x0F, 0x9A, 0x00, 0x00 }; // 16 bit code segment

    gdt[2].access = 0x93;
    gdt[2].limit_low = 0xFFFF;

    gdt[3].access = 0x93;
    gdt[3].limit_low = 0xFFFF;

    gdt_ptr_t gdtptr = { (sizeof(gdt_entry_t) * 4) - 1, to_linear(gdt) };
    idt_ptr_t idtptr = { 0, 0 };

    idt_ptr_t saved_idtptr = {};
    idt_store(&saved_idtptr);

    irq_disable();
    gdt_load(&gdtptr);
    idt_load(&idtptr);

    pmode_memcpy(regs->cx);

    idt_load(&saved_idtptr);
    irq_enable();

    regs->ah = 0x00;
    regs->CF = 0;
    regs->ZF = 1;
}

void system_memory_size(bioscall __seg_ss* const regs)
{
    uint8_t lo = cmos_read(0x17);
    uint8_t hi = cmos_read(0x18);
    regs->ax = as_uint16(hi, lo);
}

void system_enter_pmode(bioscall __seg_ss* const regs)
{
    regs->ah = 0xFF;
    regs->CF = 1;
}

void system_device_busy(bioscall __seg_ss* const regs)
{
    regs->ah = 0;
    regs->CF = 0;
}

void system_interrupt_end(bioscall __seg_ss* const regs)
{
    (void) regs;
}

void system_read_config(bioscall __seg_ss* const regs)
{
    const pointer ptr = (pointer)(void __far*)&system_config;
    regs->es = ptr.seg;
    regs->bx = ptr.off;
    
    regs->ah = 0;
    regs->CF = 0;
}