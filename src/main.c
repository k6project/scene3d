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
    VkShaderModule generator;
    VkDescriptorSetLayout generatorSetLayout;
    VkPipelineLayout generatorPipelineLayout;
} AppState;

static void initGenerator(AppState* app)
{
    memStackFramePush(app->memory);
    VkShaderModuleCreateInfo info =
    {
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .pNext = NULL, .flags = 0, .codeSize = 0, .pCode = NULL
    };
    info.pCode = sysLoadFile("/shaders/cityget.comp.spv", &info.codeSize, app->memory, MEM_STACK);
    memStackFramePop(app->memory);
    VK_ASSERT_Q(vkCreateShaderModule(gVkDev, &info, NULL, &app->generator));
    
    VkDescriptorSetLayoutBinding outBinding;
    outBinding.binding = 0;
    outBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    outBinding.descriptorCount = 1; // array size
    outBinding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
    outBinding.pImmutableSamplers = NULL;
    
    // can be shared - only a relatively small number of layouts will be used
    VkDescriptorSetLayoutCreateInfo layoutInfo;
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.pNext = NULL;
    layoutInfo.flags = 0;
    layoutInfo.bindingCount = 1;
    layoutInfo.pBindings = &outBinding;
    VK_ASSERT_Q(vkCreateDescriptorSetLayout(gVkDev, &layoutInfo, NULL, &app->generatorSetLayout));
    
    // multiple pipelines can have identical layout
    VkPushConstantRange constRange;
    constRange.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
    constRange.offset = 0;
    constRange.size = 8;
    VkPipelineLayoutCreateInfo pipelineLayoutInfo;
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.pNext = NULL;
    pipelineLayoutInfo.flags = 0;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &app->generatorSetLayout;
    pipelineLayoutInfo.pushConstantRangeCount = 1;
    pipelineLayoutInfo.pPushConstantRanges = &constRange;
    VK_ASSERT_Q(vkCreatePipelineLayout(gVkDev, &pipelineLayoutInfo, NULL, &app->generatorPipelineLayout));
}

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
    initGenerator(app);
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
    AppState* app = dataPtr;
    vkDeviceWaitIdle(gVkDev);
    
    vkDestroyPipelineLayout(gVkDev, app->generatorPipelineLayout, NULL);
    vkDestroyDescriptorSetLayout(gVkDev, app->generatorSetLayout, NULL);
    vkDestroyShaderModule(gVkDev, app->generator, NULL);
	
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
