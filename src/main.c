#include "global.h"

#include "vk_api.h"

/* 256k stack allocator */
#define MAX_STACK (1<<18)

/* 768k forward allocator */
#define MAX_FORWD ((1<<19)+MAX_STACK)

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
	HMemAlloc memory;
    const Options* options;
} AppState;

void appOnStartup(void* dataPtr)
{
    AppState* app = dataPtr;
    vkxInitialize(0, app->options, NULL);
    vkxRequestQueues(1, (VkxQueueReq[]) { { VK_QUEUE_GRAPHICS_BIT, true, &gQueueFamily, &gCmdQueue } });
    vkxCreateDeviceAndSwapchain();
    vkxCreateCommandPool(gQueueFamily, &gCmdPool);
    vkxCreateCommandBuffer(gCmdPool, VK_COMMAND_BUFFER_LEVEL_PRIMARY, 0, &gCmdBuff);
    vkxCreateSemaphore(&gFrmBegin, 0);
    vkxCreateSemaphore(&gFrmEnd, 0);
	vkxCreateFence(&gDrawFence, 0);
	gFence = gDrawFence[0];
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

void appOnShutdown(void* dataPtr)
{
    vkDeviceWaitIdle(gVkDev);
	vkxDestroyFence(gDrawFence, 0);
    vkxDestroySemaphore(gFrmEnd, 0);
    vkxDestroySemaphore(gFrmBegin, 0);
    vkxDestroyCommandBuffer(gCmdPool, 0, gCmdBuff);
    vkDestroyCommandPool(gVkDev, gCmdPool, gVkAlloc);
    vkxFinalize();
}

extern void appInitialize(HMemAlloc mem, const Options* opts, void* state);

extern bool appShouldKeepRunning(void);

int appMain(int argc, const char** argv)
{
    AppState appState;
    appState.memory = memAllocCreate(MAX_FORWD, MAX_STACK, NULL, 0);
    appState.options = argParse(argc, argv, appState.memory);
	appInitialize(appState.memory, appState.options, &appState);
	while (appShouldKeepRunning())
        renderFrame();
	return 0;
}
