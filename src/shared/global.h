#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef _MSC_VER
#include <wchar.h>
typedef wchar_t TChar;
#define STR(s) L##s
#define debugBreak() __debugbreak()
#define FORCE_INLINE __forceinline
#else
typedef char TChar;
#define STR(s) s
#define debugBreak() __builtin_debugtrap()
#define FORCE_INLINE static inline __attribute__((always_inline))
#endif

#define ALIGN16(n) ((((n-1)>>4)+1)<<4)

#define ASSERT_Q(c) if(!(c)){debugBreak();}
#define ASSERT(c,m,...) if(!(c)){appPrintf(STR(m)STR("\n"),__VA_ARGS__);debugBreak();}

typedef float Color[4];
