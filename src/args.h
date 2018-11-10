#pragma once

#include "memory.h"

#define APP_NAME_LEN 64
#define APP_NAME_MAX (APP_NAME_LEN+1)

struct Options
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

typedef struct Options Options;

const Options* argParse(int argc, const char** argv, HMemAlloc mem);

//void vkUseExtensionsAPP(const char* names[], uint32_t count);

//void vkUseLayersAPP(const char* names[], uint32_t count);
