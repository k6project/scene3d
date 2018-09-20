#pragma once

#ifdef _MSC_VER
#define debugBreak() __debugbreak()
#define FORCE_INLINE __forceinline
#else
#define debugBreak() __builtin_debugtrap()
#define FORCE_INLINE static inline __attribute__((always_inline))
#endif

#ifdef _DEBUG
#define TEST(c) if(!(c)) debugBreak();
#define CHECK(c) if(!(c)) debugBreak();
#else
#define TEST(c) {(void)(c);}
#define CHECK(c)
#endif
