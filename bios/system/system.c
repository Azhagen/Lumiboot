#include "system/system.h"
#include "system/data.h"

#include "drivers.h"
#include "attrib.h"
#include "debug.h"

void sys_detect(void)
{
    
}

void irq_enable(void)
{
    asm volatile ("sti" ::: "memory");
}

void irq_disable(void)
{
    asm volatile ("cli" ::: "memory");
}

void idt_load(idt_ptr_t __far* idtptr)
{
    asm volatile ("lidt %0" :: "m"(*idtptr) : "memory");
}

void gdt_load(gdt_ptr_t __far* gdtptr)
{
    asm volatile ("lgdt %0" :: "m"(*gdtptr) : "memory");
}

void idt_store(idt_ptr_t __far* idtptr)
{
    asm volatile ("sidt %0" : "=m"(*idtptr) :: "memory");
}

void gdt_store(gdt_ptr_t __far* gdtptr)
{
    asm volatile ("sgdt %0" : "=m"(*gdtptr) :: "memory");
}

noreturn void unreachable(void)
{
    __builtin_unreachable();
}

noreturn void panic(void)
{
    debug_puts("[BIOS] PANIC: System halted!\n\r");

    asm volatile ("cli");
    asm volatile ("hlt");
    unreachable();
}

noreturn void reboot_soft(void)
{
    asm volatile ("cli");
    asm volatile ("jmp $0xFFFF, $0x0000");
    unreachable();
}

noreturn void reboot_hard(void)
{
    // TODO: implement
    while (true);
}