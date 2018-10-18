#include "shared/args.h"
#include "shared/vk_api.h"
#include "shared/main.inl"

static void* gVkLib = NULL;
static bool gInitOk = false;


static const char* gLayers[] = { "TEST_Layer_name" };
static const unsigned int gNumLayers = sizeof(gLayers) / sizeof(const char*);
static const char* gExtensions[] = { "TEST_Extension_name" };
static const unsigned int gNumExtensions = sizeof(gExtensions) / sizeof(const char*);


static VkInstance gInstance = VK_NULL_HANDLE;
static VkPhysicalDevice gAdapter = VK_NULL_HANDLE;
static VkDevice gDevice = VK_NULL_HANDLE;

static void initialize(void* dataPtr)
{
	vkUseLayersAPP(gLayers, gNumLayers);
	vkUseExtensionsAPP(gExtensions, gNumExtensions);
    RETURN_IF_NOT(appLoadLibrary(VK_LIBRARY, &gVkLib));
    RETURN_IF_NOT(vkCreateAndInitInstanceAPP(NULL, &gInstance) == VK_SUCCESS);
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
