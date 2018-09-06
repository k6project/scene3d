#define NAPP_PRIVATE
#include "napp_private.h"

#include <string.h>
#include <stdlib.h>

NAppState GState;

static bool GIsInitialized = false;

extern bool NAppInitializeImpl(void);

bool NAppInitialize(const int argc, const char** argv)
{
	if (!GIsInitialized)
	{
		memset(&GState, 0, sizeof(GState));
		GIsInitialized = NAppInitializeImpl();
	}
	return GIsInitialized;
}

extern bool NAppCreateWindowImpl(NAppWindow* window);
extern void NAppShutdownImpl();

NAppWindow* NAppCreateWindow(const NAppWindowInfo* windowInfo)
{
    NAppWindow* window = (NAppWindow*)calloc(1, sizeof(NAppWindow));
    window->Info = *windowInfo;
    if (!GState.FirstWindow)
    {
        GState.FirstWindow = window;
    }
    if (GState.LastWindow)
    {
        GState.LastWindow->Next = window;
        window->Prev = GState.LastWindow;
    }
    GState.LastWindow = window;
    ++GState.NumWindows;
    if (GState.IsRunning)
    {
        NAppCreateWindowImpl(window);
    }
    return window;
}

void NAppWindowClosed(NAppWindow* window)
{
    window->IsVisible = false;
    window->IsActive = false;
    if (!window->IsClosed)
    {
        window->IsClosed = true;
        if (window->Prev)
        {
            window->Prev->Next = window->Next;
        }
        else
        {
            GState.FirstWindow = window->Next;
        }
        if (window->Next)
        {
            window->Next = window->Prev;
        }
        else
        {
            GState.LastWindow = window->Prev;
        }
        free(window);
        --GState.NumWindows;
        if (!GState.NumWindows)
        {
            NAppShutdownImpl();
        }
    }
}

void NAppFinalize(void)
{
	GIsInitialized = false;
}
