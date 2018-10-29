#include "shared/vk_api.h"
#include "shared/main.inl"

static uint32_t gFrameIdx = 0;
static uint32_t gQueueFamily = 0;
static VkQueue gCmdQueue = VK_NULL_HANDLE;
static VkCommandPool gCmdPool = VK_NULL_HANDLE;
static VkCommandBuffer* gCmdBuff = NULL;
static VkSemaphore* gFrmBegin = NULL;
static VkSemaphore* gFrmEnd = NULL;
static VkFence gFence = VK_NULL_HANDLE;
static VkFence* gDrawFence = NULL;

static void initialize(void* dataPtr)
{
    vkInitializeAPP(0, NULL);
    vkRequestQueuesAPP(1, (VkQueueRequest[]) { { VK_QUEUE_GCT, true, &gQueueFamily, &gCmdQueue } });
    vkCreateDeviceAndSwapchainAPP();
    VK_CMDPOOL_CREATE_INFO(poolInfo, gQueueFamily);
    VK_ASSERT(vkCreateCommandPool(gVkDev, &poolInfo, gVkAlloc, &gCmdPool), "ERROR: Failed to create command pool");
    VK_CMDBUFF_CREATE_INFO(cmdBuffInfo, gCmdPool, VK_COMMAND_BUFFER_LEVEL_PRIMARY, 0);
    vkCreateCommandBufferAPP(&cmdBuffInfo, &gCmdBuff);
    vkCreateSemaphoreAPP(&gFrmBegin, 0);
    vkCreateSemaphoreAPP(&gFrmEnd, 0);
	vkCreateFenceAPP(&gDrawFence, 0);
	gFence = gDrawFence[0];
}

static void renderFrame()
{
    uint32_t imageIdx = 0;
    vkAcquireNextImageAPP(gFrmBegin[gFrameIdx], &imageIdx);
	VkImage displayImg = gDisplayImage[imageIdx];
    VkCommandBuffer cmdBuff = gCmdBuff[imageIdx];
	vkWaitForFences(gVkDev, 1, &gFence, VK_TRUE, UINT64_MAX);
	gFence = gDrawFence[gFrameIdx];
	vkResetFences(gVkDev, 1, &gFence);
	vkResetCommandBuffer(cmdBuff, 0);
	VkCommandBufferBeginInfo beginInfo = 
	{
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO, .pNext = NULL,
		.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, .pInheritanceInfo = NULL
	};
	vkBeginCommandBuffer(cmdBuff, &beginInfo);
	VkImageMemoryBarrier clearBarrier =
	{
		.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
		.pNext = NULL,
		.srcAccessMask = 0,
		.dstAccessMask = 0,
		.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
		.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		.srcQueueFamilyIndex = gQueueFamily,
		.dstQueueFamilyIndex = gQueueFamily,
		.image = displayImg,
		.subresourceRange =
		{
			.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
			.baseMipLevel = 0,
			.levelCount = 1,
			.baseArrayLayer = 0,
			.layerCount = 1
		}
	};
	vkCmdPipelineBarrier(cmdBuff, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, NULL, 0, NULL, 1, &clearBarrier);
	VkClearColorValue* value = (float[]){0.f, 0.4f, 0.9f, 1.f};
	vkCmdClearColorImage(cmdBuff, displayImg, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, value, 1, &clearBarrier.subresourceRange);
    VkImageMemoryBarrier presentBarrier =
    {
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        .pNext = NULL,
        .srcAccessMask = 0,
        .dstAccessMask = 0,
        .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
        .srcQueueFamilyIndex = gQueueFamily,
        .dstQueueFamilyIndex = gQueueFamily,
        .image = displayImg,
        .subresourceRange =
        {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1
        }
    };
	vkCmdPipelineBarrier(cmdBuff, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0, 0, NULL, 0, NULL, 1, &presentBarrier);
	vkEndCommandBuffer(cmdBuff);
	VkPipelineStageFlags wFlags = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
	VkSubmitInfo submitInfo =
	{
		.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO, .pNext = NULL,
		.waitSemaphoreCount = 1, .pWaitSemaphores = &gFrmBegin[gFrameIdx],
		.pWaitDstStageMask = &wFlags,
		.commandBufferCount = 1, .pCommandBuffers = &cmdBuff,
		.signalSemaphoreCount = 1, .pSignalSemaphores = &gFrmEnd[gFrameIdx]
	};
	vkQueueSubmit(gCmdQueue, 1, &submitInfo, gFence);
	VkPresentInfoKHR presentInfo = 
	{
		.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR, .pNext = NULL,
		.waitSemaphoreCount = 1, .pWaitSemaphores = &gFrmEnd[gFrameIdx],
		.swapchainCount = 1, .pSwapchains = &gVkSwapchain, .pImageIndices = &imageIdx, 
		.pResults = NULL
	};
	vkQueuePresentKHR(gCmdQueue, &presentInfo);
    gFrameIdx = vkNextFrameAPP(gFrameIdx);
}

static void finalize(void* dataPtr)
{
    vkDeviceWaitIdle(gVkDev);
	vkDestroyFenceAPP(gDrawFence, 0);
    vkDestroySemaphoreAPP(gFrmEnd, 0);
    vkDestroySemaphoreAPP(gFrmBegin, 0);
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
