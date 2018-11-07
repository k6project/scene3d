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
    //AppState* app = dataPtr;
    //MemAlloc memory = memAllocCreate(0, 0);
    vkxInitialize(0, NULL);
    vkxRequestQueues(1, (VkxQueueReq[]) { { VK_QUEUE_GRAPHICS_BIT, true, &gQueueFamily, &gCmdQueue } });
    vkxCreateDeviceAndSwapchain();
    vkxCreateCommandPool(gQueueFamily, &gCmdPool);
    vkxCreateCommandBuffer(gCmdPool, VK_COMMAND_BUFFER_LEVEL_PRIMARY, 0, &gCmdBuff);
    vkxCreateSemaphore(&gFrmBegin, 0);
    vkxCreateSemaphore(&gFrmEnd, 0);
	vkxCreateFence(&gDrawFence, 0);
	gFence = gDrawFence[0];
    //app->memory = memory;
}

static void renderFrame()
{
    uint32_t imageIdx = 0;
    VkSemaphore frameBegin = gFrmBegin[gFrameIdx];
    VkSemaphore frameEnd = gFrmEnd[gFrameIdx];
    vkxAcquireNextImage(frameBegin, &imageIdx);
	VkImage displayImg = gDisplayImage[imageIdx];
    
    VkCommandBuffer cmdBuff = gCmdBuff[imageIdx];
    VkCmdBufferInfo cmdBuffInfo = { gQueueFamily, cmdBuff };
    vkWaitForFences(gVkDev, 1, &gFence, VK_TRUE, UINT64_MAX);
	gFence = gDrawFence[gFrameIdx];
	vkResetFences(gVkDev, 1, &gFence);
	vkResetCommandBuffer(cmdBuff, 0);
    vkxBeginCommandBufferOneOff(cmdBuff);
    
    vkxCmdClearColorImage(cmdBuffInfo, displayImg, VK_CLEAR_COLOR(0.f, 0.4f, 0.9f, 1.f));
    vkxCmdPreparePresent(cmdBuffInfo, displayImg);
    
	vkEndCommandBuffer(cmdBuff);
    VK_SUBMIT_INFO(submitInfo, cmdBuff, frameBegin, frameEnd);
	vkQueueSubmit(gCmdQueue, 1, &submitInfo, gFence);
    VK_PRESENT_INFO_KHR(presentInfo, imageIdx, frameEnd);
	vkQueuePresentKHR(gCmdQueue, &presentInfo);
    gFrameIdx = vkxNextFrame(gFrameIdx);
}

static void finalize(void* dataPtr)
{
    //AppState* app = dataPtr;
    vkDeviceWaitIdle(gVkDev);
	vkxDestroyFence(gDrawFence, 0);
    vkxDestroySemaphore(gFrmEnd, 0);
    vkxDestroySemaphore(gFrmBegin, 0);
    vkxDestroyCommandBuffer(gCmdPool, 0, gCmdBuff);
    vkDestroyCommandPool(gVkDev, gCmdPool, gVkAlloc);
    vkxFinalize();
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
