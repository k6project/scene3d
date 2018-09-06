#pragma once

#ifndef NAPP_PRIVATE
#error This is a private libNApp file
#endif

#include "napp.h"

struct NAppWindow
{
    NAppWindowInfo Info;
    bool IsVisible;
    bool IsActive;
    bool IsClosed;
    void* WindowImpl;
    NAppWindow* Next;
    NAppWindow* Prev;
};

typedef struct NAppState
{
    bool IsRunning;
    int NumWindows;
    NAppWindow* FirstWindow;
    NAppWindow* LastWindow;
	void* Handle;
} NAppState;

extern NAppState GState;

void NAppWindowClosed(NAppWindow* window);
