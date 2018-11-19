#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef _MSC_VER
#define sysDebugBreak() __debugbreak()
#define FORCE_INLINE __forceinline
#else
#define sysDebugBreak() __builtin_debugtrap()
#define FORCE_INLINE static inline __attribute__((always_inline))
#endif

#define INV_IDX8  0xffu8
#define INV_IDX16 0xffffu16
#define INV_IDX   0xffffffffu

#define ALIGN16(n) ((((n-1)>>4)+1)<<4)

#ifdef _DEBUG
#define ASSERT_Q(c) if(!(c)){sysDebugBreak();}
#define ASSERT(c,m,...) if(!(c)){sysPrintf(m "\n",__VA_ARGS__);sysDebugBreak();}
#define TEST_Q(c) ASSERT_Q(c)
#define TEST(c,m,...) ASSERT(c,m,__VA_ARGS__)
#else
#define ASSERT_Q(c)
#define ASSERT(c,m,...)
#define TEST_Q(c) (c)
#define TEST(c,m,...) (c)
#endif

#include "memory.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef float Color[4];
//typedef float Vec2f[2];
typedef float Vec3f[3];
typedef float Vec4f[4];

typedef union
{
    struct { float x, y; };
    struct { float u, v; };
    float ptr[2];
} Vec2f;
    
#define V2F(a,b) ((Vec2f){(a),(b)})
   
/* File I/O */
void* sysLoadFile(const char* path, size_t* size, HMemAlloc mem, MemAllocMode mode);
    
/* Handling dynamic libraries */
bool sysLoadLibrary(const char* name, void** handle);
void* sysGetLibraryProc(void* handle, const char* name);
void sysUnloadLibrary(void* handle);

/* Misc. application-level functions */
void sysPrintf(const char* fmt, ...);

#ifdef __cplusplus
}
#endif
