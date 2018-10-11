#pragma once

#ifdef _MSC_VER
#include <wchar.h>
typedef wchar_t TChar;
typedef wchar_t* CString;
#else
typedef chat TChar
typedef char* CString;
#endif

struct Options
{
    int windowWidth;
    int windowHeight;
    const CString inputFile;
};

typedef struct Options Options;

extern Options* gOptions;

void argvParse(int argc, const CString* argv);
