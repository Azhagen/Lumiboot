#include "bios.h"
#include "attrib.h"
#include "io.h"
#include "debug.h"
#include "interrupt.h"
#include "utility.h"

static void int13h_call(bioscall __seg_ss* const regs)
{
    asm volatile (
        "push %%es\n\r"
        "mov %4, %%es\n\r"
        "int $0x13\n\r"
        "pop %%es\n\r"
        : "+a"(regs->ax), "+b"(regs->bx), "+c"(regs->cx), "+d"(regs->dx), "+m"(regs->es)
        :: "memory", "cc");
}

uint8_t bios_disk_reset(uint8_t drive)
{
    bioscall regs = {0};
    regs.ah = 0;
    regs.dl = drive;
    // int13h_call(&regs);
    // intcall(0x13, &regs);
    int13h_call(&regs);
    return regs.ah;
}

uint8_t bios_disk_read_sectors(uint8_t drive, uint8_t cylinder,
    uint8_t head, uint8_t sector, uint8_t count, void __far* buffer)
{
    pointer ptr = (pointer)buffer;
    bioscall regs = {0};
    regs.ah = 0x02;
    regs.al = count;
    regs.ch = cylinder;
    regs.cl = sector;
    regs.dh = head;
    regs.dl = drive;
    regs.es = ptr.seg;
    regs.bx = ptr.off;
    int13h_call(&regs);
    // int13h_call(&regs);
    return regs.ah;
}