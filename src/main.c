#include "global.h"

#include "args.h"
#include "math_lib.h"
#include "renderer/renderer.h"

#include <string.h>

#define APP_CPU_MEM_TOTAL (1u<<22) // 3/4 forward, 1/4 stack
#define APP_CPU_MEM_FORWD ((APP_CPU_MEM_TOTAL >> 1) + (APP_CPU_MEM_TOTAL >> 2))
#define APP_CPU_MEM_STACK (APP_CPU_MEM_TOTAL - APP_CPU_MEM_FORWD)

typedef struct
{
    Options options;
	MemAlloc memory;
    Renderer renderer;
} AppState;

void AppOnStartup(void* dataPtr)
{
    Renderer rdr = NULL;
    AppState* app = dataPtr;
	//app->renderer = GfxCreateRenderer(app->memory, app->options);
}

void AppOnShutdown(void* dataPtr)
{
	AppState* app = dataPtr;
    //GfxDestroyRenderer(app->renderer);
}

extern void AppInitialize(MemAlloc mem, Options opts, void* state);

extern bool AppShouldKeepRunning(void);

int AppMain(int argc, const char** argv)
{
	AppState appState = {0};
    appState.memory = MemAllocCreate(APP_CPU_MEM_FORWD, APP_CPU_MEM_STACK, NULL, 0);
    appState.options = ArgParseCmdLine(argc, argv, appState.memory);
	AppInitialize(appState.memory, appState.options, &appState);
    while (AppShouldKeepRunning())
    {
        //ScnTickUpdate
        //GfxRenderFrame(appState.renderer, NULL);
    }
	return 0;
}
