#include "shared/vk_api.h"
#include "shared/main.inl"

static void* gVkLib = NULL;
static bool gInitOk = false;
static VkEnvironment gVkEnv;
static VkDevice gDevice = VK_NULL_HANDLE;
static VkQueue gCmdQueue = VK_NULL_HANDLE;

static bool createLogicalDevice()
{
	uint32_t familyIndex = gVkEnv.numQueueFamilies, queueCount = 1;
	for (uint32_t i = 0; i < gVkEnv.numQueueFamilies; i++)
	{
		VkBool32 canPresent = VK_FALSE;
		vkGetPhysicalDeviceSurfaceSupportKHR(gVkEnv.adapter, i, gVkEnv.surface, &canPresent);
		if (gVkEnv.queueFamilies[i].queueCount && canPresent
			&& (gVkEnv.queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT))
		{
			familyIndex = i;
			break;
		}
	}
    QTEST_RV(vkCreateAndInitDeviceAPP(gVkEnv.adapter, 1, &familyIndex, &queueCount, NULL, NULL, &gCmdQueue, &gDevice), false);
	return true;
}

static bool createSwapChain()
{
	/*gSwapChainSize = VK_SWAPCHAIN_SIZE;
	TEST_RV(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(gAdapter, gSurface, &gSurfaceCaps) == VK_SUCCESS, false, "ERROR: Failed to get surface capabilities");
	if (gSurfaceCaps.minImageCount > gSwapChainSize)
		gSwapChainSize = gSurfaceCaps.minImageCount;
	else if (gSurfaceCaps.maxImageCount < gSwapChainSize)
		gSwapChainSize = gSurfaceCaps.maxImageCount;
	VkSwapchainCreateInfoKHR createInfo;
	VK_INIT(createInfo, VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR);
	createInfo.surface = gSurface;
	createInfo.minImageCount = gSwapChainSize;*/
	return true;
}

static void initialize(void* dataPtr)
{
    if (!gInitOk)
    {
		QTEST_R(vkInitEnvironmentAPP(&gVkEnv, NULL));
		QTEST_R(createLogicalDevice());
		//QTEST_R(createSwapChain());
		//queues
		//command buffers
        gInitOk = true;
    }
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
	if (gVkEnv.instance)
	{
		if (gVkEnv.surface)
			vkDestroySurfaceKHR(gVkEnv.instance, gVkEnv.surface, NULL);
		vkDestroyInstance(gVkEnv.instance, NULL);
	}
    appUnloadLibrary(gVkEnv.library);
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
