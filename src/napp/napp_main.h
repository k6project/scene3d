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

//TODO

#elif defined(_NAPP_MACOS_)

#define NAPP_MAIN_IMPL \
	NAPP_API void NAppArgv(int, char**); \
	int main(int argc, char** argv) \
	{ \
		NAppArgv(argc, argv); \
		NAppMainImpl(); \
		return 0; \
	} \

#else // Undefined build platform

#error Undefined build platform

#endif

#define NAppMain() \
    NAppMainImpl(void); \
    NAPP_MAIN_IMPL \
    void NAppMainImpl(void)

NAPP_API bool NAppInitialize(void);

NAPP_API void NAppRun(void);
