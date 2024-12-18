#pragma once

#if !defined(__FAR)
#define __far
#endif

#if !defined(__SEG_SS)
#define __seg_ss
#endif

#if defined(NDEBUG) && defined(DEBUG)
#undef NDEBUG
#endif

#ifndef __optimize
#define __optimize(x) __attribute__((optimize(x)))
#endif

#ifndef __section
#define __section(x) __attribute__((section(x)))
#endif

#ifndef __noreturn
#define __noreturn   __attribute__((noreturn))
#endif

#ifndef __noinline
#define __noinline   __attribute__((noinline))
#endif

#define __naked      __attribute__((naked))

#ifndef __unused
#define __unused     __attribute__((unused))
#endif

#ifndef __packed
#define __packed     __attribute__((packed))
#endif

#ifndef __interrupt
#define __interrupt  __attribute__((interrupt))
#endif

#ifndef __used
#define __used  __attribute__((used))
#endif

#ifndef __regparmcall
#define __regparmcall __attribute__((regparmcall))
#endif

#ifndef __align
#define __align(x) __attribute__((aligned(x)))
#endif

#define __assume(cond) do { if (!(cond)) __builtin_unreachable(); } while (0)