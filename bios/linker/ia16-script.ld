ENTRY(reset)

MEMORY
{
    base  (rx) : ORIGIN = 0xF0000, LENGTH = 0x10000
    fixed (rx) : ORIGIN = 0xFE000, LENGTH = 0x2000
    reset (rx) : ORIGIN = 0xFFFF0, LENGTH = 0x10
}

SECTIONS
{
    .rel.sta :
    {
        *(".text!*"    ".text.*!")
        *(".fartext!*" ".fartext.*!")
        *(".data!*"    ".data.*!")
        *(".rodata!*"  ".rodata.*!")
        *(".bss!*"     ".bss.*!")
        *(".legacy!*"  ".legacy.*!")
        *(".reset!*"   ".reset.*!")
    } >base =0

    .text :
    {
        *(.text    .text$*    ".text.*[^&]")
        *(.fartext .fartext$* ".fartext.*[^&]")
    } >base =0

    .data :
    {
        *(.data .data$* ".data.*[^&]")
        *(.rodata .rodata$* ".rodata.*[^&]")
        *(.bss .bss$* ".bss.*[^&]")
    } >base =0

    .legacy :
    {
        . = 0x005B; *(.legacy.entry)
        . = 0x02C3; *(.legacy.int02h)
        . = 0x0401; *(.legacy.hddtbl)
        . = 0x06F2; *(.legacy.int19h)
        . = 0x06F5; *(.legacy.system)
        . = 0x0729; *(.legacy.comtbl)
        . = 0x0739; *(.legacy.int14h)
        . = 0x082E; *(.legacy.int16h)
        . = 0x0987; *(.legacy.int09h)
        . = 0x0C59; *(.legacy.int13h)
        . = 0x0F57; *(.legacy.int0Eh)
        . = 0x0FC7; *(.legacy.floppy)
        . = 0x0FD2; *(.legacy.int17h)
        . = 0x1065; *(.legacy.int10h)
        . = 0x10A4; *(.legacy.vidtbl)
                    *(.legacy.kbdtbl)
        . = 0x1841; *(.legacy.int12h)
        . = 0x184D; *(.legacy.int11h)
        . = 0x1859; *(.legacy.int15h)
        . = 0x1A6E; *(.legacy.pcfont)
        . = 0x1E6E; *(.legacy.int1Ah)
        . = 0x1EA5; *(.legacy.int08h)
        . = 0x1EF3; *(.legacy.vtable)
        . = 0x1F47; *(.legacy.intirq)
        . = 0x1F53; *(.legacy.intXXh)
        . = 0x1F54; *(.legacy.int05h)
    } >fixed =0

    .reset :
    {
        *(.reset .reset$* ".reset.*[^&]")
        . = 0x10;
    } >reset =0

    .rel.end :
    {
        *(".text&*"    ".text.*&")
        *(".fartext!*" ".fartext.*!")
        *(".data&*"    ".data.*&")
        *(".rodata&*"  ".rodata.*&")
        *(".bss&*"     ".bss.*&")
        *(".legacy&*"  ".legacy.*&")
        *(".reset&*"   ".reset.*&")
    } > base =0

    /DISCARD/ :
    {
        *(.comment)
        /* *(.bss*) */
        /* *(.iplt*)
        *(.got*)
        *(.igot*) */
        /* *(.rel*) */
    }
}