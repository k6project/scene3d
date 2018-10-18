#pragma once

#include "main.h"

#include "vk_api.c"

#define RETURN_IF_NOT(c) if(!(c)) return;

static VkResult vkCreateInstanceAPP()
{
    return VK_ERROR_INITIALIZATION_FAILED;
}

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

#define VK_LIBRARY "@rpath/vulkan.framework/vulkan"

int main(int argc, const TChar** argv)
{
    return appMain(argc, argv);
}

#endif
