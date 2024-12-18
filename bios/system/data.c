#include "system/data.h"

volatile bda_t __far* const bda = (void __far*)0x00000400L;

static_assert(sizeof(bda_t) == 256, "Bad BDA size!");
static_assert(sizeof(ebda_t) < 0x2000, "EBDA too large!");

const size_t BDA_SIZE  = sizeof(bda_t);
const size_t EBDA_SIZE = sizeof(ebda_t);

ebda_t __far* get_ebda(void)
{
    pointer ptr = {};
    ptr.seg = bda->ebda_segment;
    return (ebda_t __far*)ptr.ptr;
}