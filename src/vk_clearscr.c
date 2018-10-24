#include "shared/vk_api.h"
#include "shared/main.inl"

static VkEnvironment gVkEnv = NULL;
static VkDevice gDevice = VK_NULL_HANDLE;
static VkQueue gGfxQueue = VK_NULL_HANDLE;
static VkQueue gCmpQueue = VK_NULL_HANDLE;
static VkSwapchainKHR gSwapChain = VK_NULL_HANDLE;

static void initialize(void* dataPtr)
{
    VkQueue* queues[] = { &gGfxQueue, &gCmpQueue };
    QTEST_R(vkInitEnvironmentAPP(&gVkEnv, NULL));
    QTEST_R(vkRequestQueueAPP(gVkEnv, VK_QUEUE_GRAPHICS_BIT, true));
    QTEST_R(vkRequestQueueAPP(gVkEnv, VK_QUEUE_COMPUTE_BIT, false));
    QTEST_R(vkCreateDeviceAndSwapchainAPP(gVkEnv, NULL, &gDevice, &gSwapChain, queues));
}

static void renderFrame()
{
}

static void finalize(void* dataPtr)
{
	if (gDevice)
	{
		vkDeviceWaitIdle(gDevice);
		//destroy all device-dependent objects
		vkDestroyDevice(gDevice, NULL);
	}
    vkDestroyEnvironmentAPP(gVkEnv, NULL);
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
