#include "shared/vk_api.h"
#include "shared/main.inl"

static void* gVkLib = NULL;
static bool gInitOk = false;

//static const char* gLayers[] = { "TEST_Layer_name" };
//static const unsigned int gNumLayers = sizeof(gLayers) / sizeof(const char*);
//static const char* gExtensions[] = { "TEST_Extension_name" };
//static const unsigned int gNumExtensions = sizeof(gExtensions) / sizeof(const char*);

static VkInstance gInstance = VK_NULL_HANDLE;
static VkSurfaceKHR gSurface = VK_NULL_HANDLE;
static VkPhysicalDevice gAdapter = VK_NULL_HANDLE;
static VkDevice gDevice = VK_NULL_HANDLE;

static void initialize(void* dataPtr)
{
    if (!gInitOk)
    {
        //vkUseLayersAPP(gLayers, gNumLayers);
        //vkUseExtensionsAPP(gExtensions, gNumExtensions);
        VERIFY_VOID(appLoadLibrary(VK_LIBRARY, &gVkLib), "ERROR: Failed to load library");
        VERIFY_VOID_NO_MSG(vkCreateAndInitInstanceAPP(gVkLib, NULL, &gInstance));
        VERIFY_VOID_NO_MSG(vkCreateSurfaceAPP(gInstance, NULL, &gSurface));
        VERIFY_VOID(vkGetAdapterAPP(gInstance, gSurface, &gAdapter), "ERROR: No compatible graphics adapter found");
        gInitOk = true;
    }
}

static void renderFrame()
{
}

static void finalize(void* dataPtr)
{
    if (gSurface)
        vkDestroySurfaceKHR(gInstance, gSurface, NULL);
    if (gInstance)
        vkDestroyInstance(gInstance, NULL);
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
