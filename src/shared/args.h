#pragma once

#include "global.h"

#define APP_NAME_LEN 64
#define APP_NAME_MAX (APP_NAME_LEN+1)

struct Options
{
    int windowWidth;
    int windowHeight;
    int isFullscreen;
	const TChar* windowTitle;
    const TChar* inputFile;
	TChar appName[APP_NAME_MAX];
	const char** extensions;
	const char** layers;
	uint32_t numExtensions;
	uint32_t numLayers;
};

typedef struct Options Options;

extern const Options* gOptions;

void argvParse(int argc, const TChar** argv); // pass memory allocator

void vkUseExtensionsAPP(const char* names[], uint32_t count);

void vkUseLayersAPP(const char* names[], uint32_t count);
