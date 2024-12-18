#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdnoreturn.h>

// #include "attrib.h"
// #include "sys.h"
#include "debug.h"
#include "system/system.h"
#include "io.h"
// #include "attrib.h"

#if defined(__SSP__) || defined(__SSP_ALL__)  || \
    defined(__SSP_STRONG__) || defined(__SSP_EXPLICIT__)

const uintptr_t const __stack_chk_guard = 0xDEAD;

void __stack_chk_fail(void)
{
    debug_puts("\n\r*** Stack smashing detected! ***\n\r");
    panic();
}

void __stack_chk_fail_local(void)
{
    __stack_chk_fail();
}

void __chk_fail(void)
{
    debug_puts("\n\r*** Buffer overflow detected! ***\n\r");
    panic();
}

#endif