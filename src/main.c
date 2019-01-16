#include <global.h>
#include <common/args.h>
#include <common/vecmath.h>
#include <renderer/renderer.h>

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
    AppState* app = dataPtr;
    //app->scene = ScnCreateScene();
	//app->renderer = GfxCreateRenderer(app->memory, app->options);
    //Material cubeMaterial = GfxCreateMaterial(?????);
    //Mesh cubeMesh = GfxCreateMesh(CUBE_MESH_NUM_VERTICES, CUBE_MESH_NUM_INDICES, CUBE_MESH_VERTICES, CUBE_MESH_INDICES);
    //ScnSetCamera(????);
    //SceneNode cube = ScnAddNode(app->scene, NULL, cubeMesh, cubeMaterial);
    //ScnNodeTranslate()
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
        //ScnRotateNode(cube, rQuat);
        //GfxRenderFrame(appState.renderer, appState.scene);
    }
	return 0;
}
