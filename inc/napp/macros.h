#pragma once

#ifdef __cplusplus
#ifndef _NAPP_WRL_
#define NAPP_API extern "C"
#else
#define NAPP_API
#endif
#else
#define NAPP_API
#endif

#define ALIGNED(n,b) ((((n-1)/b)+1)*b)
