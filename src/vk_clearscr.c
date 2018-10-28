#include "shared/vk_api.h"
#include "shared/main.inl"

static void initialize(void* dataPtr)
{
    vkInitialize(0);
    /*
    VkQueue* queues[] = { &gGfxQueue, &gCmpQueue };
    QTEST_R(vkInitEnvironmentAPP(&gVkEnv, NULL));
    QTEST_R(vkRequestQueueAPP(gVkEnv, VK_QUEUE_GRAPHICS_BIT, true));
    QTEST_R(vkRequestQueueAPP(gVkEnv, VK_QUEUE_COMPUTE_BIT, false));
    QTEST_R(vkCreateDeviceAndSwapchainAPP(gVkEnv, NULL, &gDevice, &gSwapChain, queues));
     */
}

static void renderFrame()
{
    /*VkImageMemoryBarrier clearBarrier =
    {
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        .pNext = NULL,
        .srcAccessMask = 0,
        .dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
        .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        .srcQueueFamilyIndex = 0,
        .dstQueueFamilyIndex = 0,
        .image = VK_NULL_HANDLE,
        .subresourceRange =
        {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1
        }
    };*/
    
    //
    
}

static void finalize(void* dataPtr)
{
	/*if (gDevice)
	{
		vkDeviceWaitIdle(gDevice);
        //destroy all device-dependent objects
        vkDestroySwapchainKHR(gDevice, gSwapChain, NULL);
		vkDestroyDevice(gDevice, NULL);
	}
    vkDestroyEnvironmentAPP(gVkEnv, NULL);*/
    vkFinalize();
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
