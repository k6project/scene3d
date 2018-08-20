#pragma once

#if defined(_MSC_VER)
#include <windows.h>
#endif

C_API EResult AppStartup();

C_API EResult AppIsFinished();

C_API void AppPollEvents();

C_API void AppShutdown();

C_API void AppMessage(const char* format, ...);
