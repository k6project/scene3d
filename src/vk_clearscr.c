#include "shared/vk_api.h"
#include "shared/main.inl"

static uint32_t gQueueFamily = 0;
static VkQueue gCmdQueue = VK_NULL_HANDLE;
static VkCommandPool gCmdPool = VK_NULL_HANDLE;
static VkCommandBuffer* gCmdBuff = NULL;

static void initialize(void* dataPtr)
{
    vkInitializeAPP(0, NULL);
    vkRequestQueuesAPP(1, (VkQueueRequest[]) { { VK_QUEUE_GCT, true, &gQueueFamily, &gCmdQueue } });
    vkCreateDeviceAndSwapchainAPP();
    VK_CMDPOOL_CREATE_INFO(poolInfo, gQueueFamily);
    VK_ASSERT(vkCreateCommandPool(gVkDev, &poolInfo, gVkAlloc, &gCmdPool), "ERROR: Failed to create command pool");
    VK_CMDBUFF_CREATE_INFO(cmdBuffInfo, gCmdPool, VK_COMMAND_BUFFER_LEVEL_PRIMARY, 0);
    vkCreateCommandBufferAPP(&cmdBuffInfo, &gCmdBuff);
}

static void renderFrame()
{
    uint32_t imageIdx = 0;
    //acquire image
    VkCommandBuffer cmdBuff = gCmdBuff[imageIdx];
    //reset command buffer
    //record commands onto cmdBuff
    //end command 
    //submit
    //present

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
    vkDeviceWaitIdle(gVkDev);
    vkDestroyCommandBufferAPP(gCmdPool, 0, gCmdBuff);
    vkDestroyCommandPool(gVkDev, gCmdPool, gVkAlloc);
    vkFinalizeAPP();
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
