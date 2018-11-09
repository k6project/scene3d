#pragma once

#include "main.h"

#ifndef NO_VULKAN
#include "vk_api.c"
#endif

int appMain(int argc, const TChar** argv);

#ifdef _MSC_VER

#include <windows.h>

#include "win32.c"

int CALLBACK WinMain(HINSTANCE inst, HINSTANCE prev, LPSTR cmd, int show)
{
	int argc = 0;
	const TChar** argv = (const TChar**)CommandLineToArgvW(GetCommandLine(), &argc);
    appMain(argc, argv);
	LocalFree((HLOCAL)argv);
	return show;
}

#else // OSX

int main(int argc, const TChar** argv)
{
    int retval = appMain(argc, argv);
    return retval;
}

#endif
