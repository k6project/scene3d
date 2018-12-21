#pragma once

#include "memory.h"

#define APP_NAME_LEN 64
#define APP_NAME_MAX (APP_NAME_LEN+1)

struct OptionsImpl
{
    int windowWidth;
    int windowHeight;
    int isFullscreen;
	const char* windowTitle;
    const char* inputFile;
	char appName[APP_NAME_MAX];
	const char** extensions;
	const char** layers;
	uint32_t numExtensions;
	uint32_t numLayers;
};

typedef const struct OptionsImpl* Options;

Options arg_ParseCmdLine(int argc, const char** argv, MemAlloc mem);
