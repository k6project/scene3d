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

#include <stdbool.h>

typedef void*(*NAppStartupProc)();
typedef void(*NAppShutdownProc)(void*);
typedef void(*NAppUpdateProc)(void*);

NAPP_API void NAppSetFullscreen(bool value);

NAPP_API void NAppSetViewSize(int width, int height);

/*******************/

typedef struct NAppWindowInfo
{
    const char* Title;
	int XOrg, YOrg, Width, Height;
	bool IsFullscreen;
} NAppWindowInfo;

typedef struct NAppWindow NAppWindow;

NAPP_API NAppWindow* NAppCreateWindow(const NAppWindowInfo* windowInfo);

NAPP_API const NAppWindowInfo* NAppGetWindowInfo(NAppWindow* window);

NAPP_API void NAppDestroyWindow(NAppWindow* window);

NAPP_API void NAppPollEvents(NAppWindow* window);

NAPP_API bool NAppIsFinished(void);

NAPP_API void NAppFinalize(void);
