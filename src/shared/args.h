#pragma once

struct Options
{
    int windowWidth;
    int windowHeight;
    const char* inputFile;
};

typedef struct Options Options;

extern Options* gOptions;

void argvParse(int argc, const char** argv);
