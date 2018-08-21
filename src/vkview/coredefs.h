#pragma once

#ifdef __cplusplus
#define C_API extern "C"
#else
#define C_API extern
#endif

typedef enum EResult
{
	RES_NO_ERROR, 
	RES_UNKNOWN_ERROR, 
	RES_PLATFORM_ERROR,
	RES_GRAPHICS_ERROR,
	RES_NOT_SUPPORTED,
	RES_APP_SHUTDOWN
} EResult;
