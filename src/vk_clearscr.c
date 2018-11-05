#include "shared/main.inl"
#include "shared/vk_api.h"

static uint32_t gFrameIdx = 0;
static uint32_t gQueueFamily = 0;
static VkQueue gCmdQueue = VK_NULL_HANDLE;
static VkCommandPool gCmdPool = VK_NULL_HANDLE;
static VkCommandBuffer* gCmdBuff = NULL;
static VkSemaphore* gFrmBegin = NULL;
static VkSemaphore* gFrmEnd = NULL;
static VkFence gFence = VK_NULL_HANDLE;
static VkFence* gDrawFence = NULL;

typedef struct
{
    /*MemAlloc*/void* memory;
} AppState;

static void initialize(void* dataPtr)
{
    AppState* app = dataPtr;
    //MemAlloc memory = memAllocCreate(0, 0);
    vkInitializeAPP(0, NULL);
    vkRequestQueuesAPP(1, (VkQueueRequest[]) { { VK_QUEUE_GRAPHICS_BIT, true, &gQueueFamily, &gCmdQueue } });
    vkCreateDeviceAndSwapchainAPP();
    VK_CMDPOOL_CREATE_INFO(poolInfo, gQueueFamily);
    vkCreateCommandPool(gVkDev, &poolInfo, gVkAlloc, &gCmdPool);
    VK_CMDBUFF_CREATE_INFO(cmdBuffInfo, gCmdPool, VK_COMMAND_BUFFER_LEVEL_PRIMARY, 0);
    vkCreateCommandBufferAPP(&cmdBuffInfo, &gCmdBuff);
    vkCreateSemaphoreAPP(&gFrmBegin, 0);
    vkCreateSemaphoreAPP(&gFrmEnd, 0);
	vkCreateFenceAPP(&gDrawFence, 0);
	gFence = gDrawFence[0];
    //app->memory = memory;
}

static void renderFrame()
{
    uint32_t imageIdx = 0;
    VkSemaphore frameBegin = gFrmBegin[gFrameIdx];
    VkSemaphore frameEnd = gFrmEnd[gFrameIdx];
    vkAcquireNextImageAPP(frameBegin, &imageIdx);
	VkImage displayImg = gDisplayImage[imageIdx];
    VkCommandBuffer cmdBuff = gCmdBuff[imageIdx];
    VkCmdBufferInfo cmdBuffInfo = { gQueueFamily, cmdBuff };
	vkWaitForFences(gVkDev, 1, &gFence, VK_TRUE, UINT64_MAX);
	gFence = gDrawFence[gFrameIdx];
	vkResetFences(gVkDev, 1, &gFence);
	vkResetCommandBuffer(cmdBuff, 0);
    vkBeginCommandBufferOneOffAPP(cmdBuff);
    vkCmdClearColorImageAPP(cmdBuffInfo, displayImg, VK_CLEAR_COLOR(0.f, 0.4f, 0.9f, 1.f));
    vkCmdPreparePresentAPP(cmdBuffInfo, displayImg);
	vkEndCommandBuffer(cmdBuff);
    VK_SUBMIT_INFO(submitInfo, cmdBuff, frameBegin, frameEnd);
	vkQueueSubmit(gCmdQueue, 1, &submitInfo, gFence);
    VK_PRESENT_INFO_KHR(presentInfo, imageIdx, frameEnd);
	vkQueuePresentKHR(gCmdQueue, &presentInfo);
    gFrameIdx = vkNextFrameAPP(gFrameIdx);
}

static void finalize(void* dataPtr)
{
    AppState* app = dataPtr;
    vkDeviceWaitIdle(gVkDev);
	vkDestroyFenceAPP(gDrawFence, 0);
    vkDestroySemaphoreAPP(gFrmEnd, 0);
    vkDestroySemaphoreAPP(gFrmBegin, 0);
    vkDestroyCommandBufferAPP(gCmdPool, 0, gCmdBuff);
    vkDestroyCommandPool(gVkDev, gCmdPool, gVkAlloc);
    vkFinalizeAPP();
    //memAllocRelease(app->memory);
    //app->memory = NULL;
}

int appMain(int argc, const TChar** argv)
{
    AppState appState;
	argvParse(argc, argv);
	static AppCallbacks cbMap =
	{
		.beforeStart = &initialize,
        .beforeStop = &finalize
	};
	appInitialize(&cbMap, &appState);
	while (appShouldKeepRunning())
        renderFrame();
	return 0;
}
