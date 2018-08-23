#pragma once

#ifdef __cplusplus
#define NAPP_API extern "C"
#else
#define NAPP_API
#endif

#include <stdbool.h>

typedef struct NAppWindowInfo
{
	int XOrg, YOrg, Width, Height;
	bool IsFullscreen;
} NAppWindowInfo;

typedef struct NAppWindow NAppWindow;

NAPP_API bool NAppInitialize(const int argc, const char** argv);

NAPP_API NAppWindow* NAppCreateWindow(const NAppWindowInfo* windowInfo);

NAPP_API const NAppWindowInfo* NAppGetWindowInfo(NAppWindow* window);

NAPP_API void NAppDestroyWindow(NAppWindow* window);

NAPP_API void NAppPollEvents(NAppWindow* window);

NAPP_API bool NAppIsFinished();

NAPP_API void NAppFinalize();
