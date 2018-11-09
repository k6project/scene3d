#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef _MSC_VER
#define debugBreak() __debugbreak()
#define FORCE_INLINE __forceinline
#else
#define debugBreak() __builtin_debugtrap()
#define FORCE_INLINE static inline __attribute__((always_inline))
#endif

#define ALIGN16(n) ((((n-1)>>4)+1)<<4)

#ifdef _DEBUG
#define ASSERT_Q(c) if(!(c)){debugBreak();}
#define ASSERT(c,m,...) if(!(c)){appPrintf(m "\n",__VA_ARGS__);debugBreak();}
#define TEST_Q(c) ASSERT_Q(c)
#define TEST(c,m,...) ASSERT(c,m,__VA_ARGS__)
#else
#define ASSERT_Q(c)
#define ASSERT(c,m,...)
#define TEST_Q(c) (c)
#define TEST(c,m,...) (c)
#endif

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct AppCallbacks
{
	void(*beforeStart)(void*);
	void(*beforeStop)(void*);
} AppCallbacks;

typedef float Color[4];
typedef float Vec2f[2];
typedef float Vec3f[3];
typedef float Vec4f[4];
   
/* Handling dynamic libraries */
bool appLoadLibrary(const char* name, void** handle);
void* appGetLibraryProc(void* handle, const char* name);
void appUnloadLibrary(void* handle);

/* Misc. application-level functions */
void appGetName(char* buff, size_t max);
void appPrintf(const char* fmt, ...);

#ifdef __cplusplus
}
#endif
