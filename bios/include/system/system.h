#pragma once

#define CPU_8086 0
#define CPU_8088 1
#define CPU_186  2
#define CPU_188  3
#define CPU_V20  4
#define CPU_V30  5
#define CPU_286  6
#define CPU_386  7

#ifndef __ASSEMBLER__

#include <stdnoreturn.h>
#include <stdbool.h>

#include "gdt.h"

noreturn void unreachable(void);
noreturn void panic(void);
noreturn void reboot_soft(void);
noreturn void reboot_hard(void);

void cpu_detect(void);
void sys_detect(void);
void mem_detect(void);

void irq_enable(void);
void irq_disable(void);

void idt_load(idt_ptr_t __far* idtptr);
void gdt_load(gdt_ptr_t __far* gdtptr);
void idt_store(idt_ptr_t __far* idtptr);
void gdt_store(gdt_ptr_t __far* gdtptr);

void rom_init(void);

#endif