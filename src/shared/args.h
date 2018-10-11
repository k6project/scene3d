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
    const TChar* inputFile;
};

typedef struct Options Options;

extern Options* gOptions;

void argvParse(int argc, const TChar** argv);
