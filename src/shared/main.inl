#pragma once

#include "main.h"

int appMain(int argc, const TChar** argv);

#ifdef _MSC_VER

#include <windows.h>

#include "win32.c"

int CALLBACK WinMain(HINSTANCE inst, HINSTANCE prev, LPSTR cmd, int show)
{
	int argc = 0;
	const TChar** argv = CommandLineToArgvW(GetCommandLineW(), &argc);
	appMain(argc, argv);
	LocalFree(argv);
	return show;
}

#else // OSX or Linux

int main(int argc, const TChar** argv)
{
    return appMain(argc, argv);
}

#endif
