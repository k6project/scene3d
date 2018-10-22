#pragma once

#include "main.h"

#include "vk_api.c"

int appMain(int argc, const TChar** argv);

#ifdef _MSC_VER

#include <windows.h>

#include "win32.c"

#define VK_LIBRARY L"vulkan-1.dll"

int CALLBACK WinMain(HINSTANCE inst, HINSTANCE prev, LPSTR cmd, int show)
{
	int argc = 0;
	const TChar** argv = CommandLineToArgvW(GetCommandLine(), &argc);
    appMain(argc, argv);
	LocalFree((HLOCAL)argv);
	return show;
}

#else // OSX

#define VK_LIBRARY "@rpath/libvulkan.1.dylib"

int main(int argc, const TChar** argv)
{
    int retval = appMain(argc, argv);
    return retval;
}

#endif
