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

#ifdef __cplusplus
extern "C"
{
#endif

/*typedef uint8_t* MemStack[2];

#define memStackBeginFrame(n, s) const uint8_t* const n##_stck_frm_ = s[0]
#define memStackEndFrame(n, s) s[0] = n##_stck_frm_
void* memStackInit(MemStack stack, void* mem, size_t size);
void* memStackAllocate(MemStack stack, size_t bytes);

struct MemAlloc_;
typedef struct MemAlloc_* MemAlloc;
MemAlloc memAllocCreate(size_t forwd, size_t stack);*/


typedef float Color[4];

#ifdef __cplusplus
}
#endif
