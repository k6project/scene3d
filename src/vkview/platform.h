#pragma once

#if defined(_MSC_VER)
#include <windows.h>
#endif

typedef struct AppWindow
{
#if defined(_MSC_VER)
	HWND Handle;
#endif
	int Width, Height;
} AppWindow;

C_API EResult AppStartup();

C_API EResult AppIsFinished();

C_API void AppPollEvents();

C_API void AppShutdown();

C_API void AppMessage(const char* format, ...);

C_API const AppWindow* AppGetWindow();
