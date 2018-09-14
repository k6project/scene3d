#pragma once

#include "napp.h"

#if defined(_NAPP_WINAPI_)

#define NAPP_MAIN_IMPL \
	NAPP_API void NAppArgv(LPSTR); \
    int CALLBACK WinMain(HINSTANCE inst, HINSTANCE prev, LPSTR cmd, int show) \
	{ \
		NAppArgv(cmd); \
		NAppMainImpl(); \
		return show; \
	} \

#elif defined(_NAPP_WRL_)

#define NAPP_MAIN_IMPL \
    NAPP_API void NAppArgv(Platform::Array<Platform::String^>^); \
	[Platform::MTAThread] \
	int main(Platform::Array<Platform::String^>^ argv) \
	{ \
		NAppArgv(argv); \
        NAppMainImpl(); \
        return 0; \
    } \

#elif defined(_NAPP_MACOS_)

#define NAPP_MAIN_IMPL \
	NAPP_API void NAppArgv(int, char**); \
	int main(int argc, char** argv) \
	{ \
		NAppArgv(argc, argv); \
		NAppMainImpl(); \
		return 0; \
	} \

#else

#error Undefined build platform

#endif

#define NAppMain() \
    NAppMainImpl(void); \
    NAPP_MAIN_IMPL \
    void NAppMainImpl(void)

NAPP_API bool NAppInitialize(void);

NAPP_API void NAppRun(void);
