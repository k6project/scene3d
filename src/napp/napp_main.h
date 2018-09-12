#pragma once

#include "napp.h"

NAPP_API bool NAppInitialize();

NAPP_API int NAppRun(void);

#if defined(_NAPP_WINAPI_)

#define NAPP_MAIN_IMPL \
	extern void NAppArgv(LPSTR); \
    int CALLBACK WinMain(HINSTANCE inst, HINSTANCE prev, LPSTR cmd, int show) \
	{ \
		NAppArgv(cmd); \
		NAppMainImpl(); \
		return show; \
	} \

#elif defined(_NAPP_WRL_)

//TODO

#elif defined(_NAPP_MAC_)

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
    NAppMainImpl(); \
    NAPP_MAIN_IMPL \
    void NAppMainImpl()
