#pragma once

#include "macros.h"

#if defined(_NAPP_WINAPI_)

#include <windows.h>
#define NAPP_MAIN_IMPL \
	extern "C" void napp_argv(LPSTR); \
    int CALLBACK WinMain(HINSTANCE inst, HINSTANCE prev, LPSTR cmd, int show) \
	{ \
		napp_argv(cmd); \
		napp_main_(); \
		return show; \
	} \

#elif defined(_NAPP_WRL_)

#define NAPP_MAIN_IMPL \
    extern void napp_argv(Platform::Array<Platform::String^>^); \
	[Platform::MTAThread] \
	int main(Platform::Array<Platform::String^>^ argv) \
	{ \
		napp_argv(argv); \
        napp_main_(); \
        return 0; \
    } \

#elif defined(_NAPP_MACOS_)

#define NAPP_MAIN_IMPL \
	extern "C" void napp_argv(int, char**); \
	int main(int argc, char** argv) \
	{ \
		napp_argv(argc, argv); \
		napp_main_(); \
		return 0; \
	} \

#else

#error Undefined build platform

#endif

#define NAppMain() \
    NAppMainImpl(void); \
    NAPP_MAIN_IMPL \
    void NAppMainImpl(void)

#define napp_main() \
    napp_main_(void); \
    NAPP_MAIN_IMPL \
    void napp_main_(void)

NAPP_API bool napp_initialize(void);
NAPP_API void napp_run(void);
