#pragma once

#include "main.h"

int appMain(int argc, const char** argv);

#ifdef MSC_VER

// declare WinMain

#else // OSX or Linux

int main(int argc, const char** argv)
{
    return appMain(argc, argv);
}

#endif
