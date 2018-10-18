#include "shared/args.h"
#include "shared/vk_api.h"
#include "shared/main.inl"

static void* gVkLib = NULL;
static bool gInitOk = false;

static void initialize(void* dataPtr)
{
    RETURN_IF_NOT(appLoadLibrary(VK_LIBRARY, &gVkLib));
    //RETURN_IF_NOT(vkCreateInstanceAPP() == VK_SUCCESS);
    gInitOk = true;
}

static void renderFrame()
{
}

static void finalize(void* dataPtr)
{
    appUnloadLibrary(gVkLib);
}

int appMain(int argc, const TChar** argv)
{
	argvParse(argc, argv);
	static AppCallbacks cbMap =
	{
		.beforeStart = &initialize,
        .beforeStop = &finalize
	};
	appInitialize(&cbMap, NULL);
	while (appShouldKeepRunning())
    {
		appPollEvents();
        renderFrame();
    }
	return 0;
}
