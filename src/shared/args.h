#pragma once

#ifdef _MSC_VER
#include <wchar.h>
typedef wchar_t TChar;
#else
typedef char TChar;
#endif

struct Options
{
    int windowWidth;
    int windowHeight;
    int isFullscreen;
    const TChar* inputFile;
};

typedef struct Options Options;

extern const Options* gOptions;

void argvParse(int argc, const TChar** argv);
