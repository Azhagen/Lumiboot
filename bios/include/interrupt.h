#pragma once

#include <stdint.h>
#include <stddef.h>

#include "attrib.h"

typedef uint16_t word_t;
typedef uint8_t  byte_t;

#define FLAGS union {     \
    word_t flags;         \
    struct {              \
        word_t CF : 1;    \
        word_t _0 : 1;    \
        word_t PF : 1;    \
        word_t _1 : 1;    \
        word_t AF : 1;    \
        word_t _2 : 1;    \
        word_t ZF : 1;    \
        word_t SF : 1;    \
        word_t TF : 1;    \
        word_t IF : 1;    \
        word_t DF : 1;    \
        word_t OF : 1;    \
        word_t IOPL : 2;  \
        word_t NT : 1;    \
        word_t MD : 1;    \
        };                \
    };

#define WORD(w) union { \
    word_t w##x;        \
    struct {            \
        byte_t w##l;    \
        byte_t w##h;    \
        };              \
    };

typedef struct bioscall
{
    WORD(a);
    WORD(b);
    WORD(c);
    WORD(d);
    word_t si;
    word_t di;
    word_t es;
    FLAGS;
} bioscall;

struct registers
{
    FLAGS;
    word_t ds;
    word_t es;
    word_t di;
    word_t si;
    word_t bp;
    word_t sp;
    WORD(b);
    WORD(d);
    WORD(c);
    WORD(a);
};

typedef struct registers registers_t;

struct regs 
{
    WORD(a);
    WORD(b);
    WORD(c);
    WORD(d);
    FLAGS;
};

typedef struct regs intregs;

void interrupt_install_all(void);
void interrupt_install(size_t num, uint16_t addr);
