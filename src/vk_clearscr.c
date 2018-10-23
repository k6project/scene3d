#include "shared/vk_api.h"
#include "shared/main.inl"

static void* gVkLib = NULL;
static bool gInitOk = false;

static VkInstance gInstance = VK_NULL_HANDLE;
static VkSurfaceKHR gSurface = VK_NULL_HANDLE;
static VkPhysicalDevice gAdapter = VK_NULL_HANDLE;
static VkQueueFamilyProperties* gQueueFamilyProps = NULL;
static VkSurfaceCapabilitiesKHR gSurfaceCaps;
static uint32_t gNumQueueFamilies = 0;
static uint32_t gSwapChainSize = 0;
static VkDevice gDevice = VK_NULL_HANDLE;
static VkQueue gCmdQueue = VK_NULL_HANDLE;

static bool initVulkanApi()
{
	TEST_RV(appLoadLibrary(VK_LIBRARY, &gVkLib), false, "ERROR: Failed to load library");
	QTEST_RV(vkCreateAndInitInstanceAPP(gVkLib, NULL, &gInstance), false);
	QTEST_RV(vkCreateSurfaceAPP(gInstance, NULL, &gSurface), false);
	TEST_RV(vkGetAdapterAPP(gInstance, gSurface, &gAdapter), false, "ERROR: No compatible graphics adapter found");
	return true;
}

static bool createLogicalDevice()
{
	TEST_RV(vkGetQueueFamiliesAPP(gAdapter, &gNumQueueFamilies, &gQueueFamilyProps), false, "ERROR: Failed to enum queue families");
    float priority = 1.f;
	uint32_t familyIndex = gNumQueueFamilies, queueCount = 1;
	for (uint32_t i = 0; i < gNumQueueFamilies; i++)
	{
		VkBool32 canPresent = VK_FALSE;
		vkGetPhysicalDeviceSurfaceSupportKHR(gAdapter, i, gSurface, &canPresent);
		if (gQueueFamilyProps[i].queueCount && canPresent
			&& (gQueueFamilyProps[i].queueFlags & VK_QUEUE_GRAPHICS_BIT))
		{
			familyIndex = i;
			break;
		}
	}
    QTEST_RV(vkCreateAndInitDeviceAPP(gAdapter, 1, &familyIndex, &queueCount, &priority, NULL, NULL, &gDevice), false);
	return true;
}

static bool createSwapChain()
{
	gSwapChainSize = VK_SWAPCHAIN_SIZE;
	TEST_RV(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(gAdapter, gSurface, &gSurfaceCaps) == VK_SUCCESS, false, "ERROR: Failed to get surface capabilities");
	if (gSurfaceCaps.minImageCount > gSwapChainSize)
		gSwapChainSize = gSurfaceCaps.minImageCount;
	else if (gSurfaceCaps.maxImageCount < gSwapChainSize)
		gSwapChainSize = gSurfaceCaps.maxImageCount;
	VkSwapchainCreateInfoKHR createInfo;
	VK_INIT(createInfo, VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR);
	createInfo.surface = gSurface;
	createInfo.minImageCount = gSwapChainSize;
	return true;
}

static void initialize(void* dataPtr)
{
    if (!gInitOk)
    {
		QTEST_R(initVulkanApi());
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
        vkDestroyDevice(gDevice, NULL);
    if (gSurface)
        vkDestroySurfaceKHR(gInstance, gSurface, NULL);
    if (gInstance)
        vkDestroyInstance(gInstance, NULL);
	free(gQueueFamilyProps);
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
