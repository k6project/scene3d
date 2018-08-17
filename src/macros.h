#pragma once

#ifdef __cplusplus

#define C_API extern "C"

#else // __cplusplus

#define C_API extern

#endif // __cplusplus

#ifdef _MSC_VER

#define BIG_UINT(n) n

#else // _MSC_VER

#define BIG_UINT(n) n##llu

#endif // _MSC_VER
