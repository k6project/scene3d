#include "global.h"

#include "args.h"
#include "render.h"
#include "vk_api.h"

#include <string.h>

/* 256k stack allocator */
#define MAX_STACK (1<<18)

/* 768k forward allocator */
#define MAX_FORWD ((1<<19)+MAX_STACK)

#define MAX_DESC_BUFFER 8
#define MAX_DESC_IMAGE  0
#define MAX_DESC_SETS   8

typedef struct
{
	HMemAlloc memory;
    const Options* options;
    HRenderer renderer;
#if 0
    VkDescriptorPool descPool;
    
    VkShaderModule generator;
    VkDescriptorSetLayout generatorSetLayout;
    VkPipelineLayout generatorPipelineLayout;
    VkPipeline generatorPipeline;
    VkBuffer generatorOutput;
    VkDeviceMemory generatorMemory;
    VkDescriptorSet generatorDescr;
    void* generatorSetup;
    uint32_t generatorSetupLen;
#endif
} AppState;

static void initGenerator(AppState* app, Vec2f size, uint32_t rows, uint32_t cols)
{
#if 0
    struct {
        Vec3f gridStep;
        uint32_t numVerts;
        Vec2f gridRange;
        uint32_t minHeight;
        uint32_t maxHeight;
    } genParams =
    {
        {size.x / ((float)cols), size.y / ((float)rows), 0.0f},
        6, {size.x * 0.5f, size.y * 0.5f}, 2, 9
    };
    {
        memStackFramePush(app->memory);
        VkShaderModuleCreateInfo info = {0};
        info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        info.pCode = sysLoadFile("/shaders/cityget.comp.spv", &info.codeSize, app->memory, MEM_STACK);
        VK_ASSERT_Q(vkCreateShaderModule(gVkDev, &info, NULL, &app->generator));
        memStackFramePop(app->memory);
    }
    {
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
    }
    {
        // multiple pipelines can have identical layout
        VkPushConstantRange constRange;
        constRange.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
        constRange.offset = 0;
        constRange.size = sizeof(genParams);
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
    {
        uint32_t localSize[] = { cols, rows };
        VkSpecializationMapEntry specMap[2];
        specMap[0].constantID = 0;
        specMap[0].offset = 0;
        specMap[0].size = sizeof(uint32_t);
        specMap[1].constantID = 1;
        specMap[1].offset = specMap[0].size & UINT32_MAX;
        specMap[1].size = sizeof(uint32_t);
        VkSpecializationInfo specInfo;
        specInfo.mapEntryCount = 2;
        specInfo.pMapEntries = specMap;
        specInfo.dataSize = sizeof(localSize);
        specInfo.pData = localSize;
        VkComputePipelineCreateInfo createInfo;
        createInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
        createInfo.pNext = NULL;
        createInfo.flags = 0;
        createInfo.stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        createInfo.stage.pNext = NULL;
        createInfo.stage.flags = 0;
        createInfo.stage.stage = VK_SHADER_STAGE_COMPUTE_BIT;
        createInfo.stage.module = app->generator;
        createInfo.stage.pName = VK_SHADER_MAIN;
        createInfo.stage.pSpecializationInfo = &specInfo;
        createInfo.layout = app->generatorPipelineLayout;
        createInfo.basePipelineHandle = VK_NULL_HANDLE;
        createInfo.basePipelineIndex = -1;
        VK_ASSERT_Q(vkCreateComputePipelines(gVkDev, VK_NULL_HANDLE, 1, &createInfo, NULL, &app->generatorPipeline));
    }
    {
        VkBufferCreateInfo buffInfo;
        buffInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        buffInfo.pNext = NULL;
        buffInfo.flags = 0;
        buffInfo.size = (2 * sizeof(Vec4f)) * 6 * cols * rows;
        buffInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
        buffInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        buffInfo.queueFamilyIndexCount = 1;
        buffInfo.pQueueFamilyIndices = &gQueueFamily;
        VK_ASSERT_Q(vkCreateBuffer(gVkDev, &buffInfo, NULL, &app->generatorOutput));
        app->generatorMemory = vkxMallocBuffer(app->generatorOutput, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    }
    {
        VkDescriptorSetAllocateInfo info;
        info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        info.pNext = NULL;
        info.descriptorPool = app->descPool;
        info.descriptorSetCount = 1;
        info.pSetLayouts = &app->generatorSetLayout;
        VK_ASSERT_Q(vkAllocateDescriptorSets(gVkDev, &info, &app->generatorDescr));
    }
    {
        VkDescriptorBufferInfo info;
        info.buffer = app->generatorOutput;
        info.offset = 0;
        info.range = (2 * sizeof(Vec4f)) * genParams.numVerts * cols * rows;
        VkWriteDescriptorSet update;
        update.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        update.pNext = NULL;
        update.dstSet = app->generatorDescr;
        update.dstBinding = 0;
        update.dstArrayElement = 0;
        update.descriptorCount = 1;
        update.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        update.pImageInfo = NULL;
        update.pBufferInfo = &info;
        update.pTexelBufferView = NULL;
        vkUpdateDescriptorSets(gVkDev, 1, &update, 0, NULL);
    }
    {
        app->generatorSetupLen = sizeof(genParams);
        app->generatorSetup = memForwdAlloc(app->memory, sizeof(genParams));
        memcpy(app->generatorSetup, &genParams, sizeof(genParams));
    }
#endif
}

void appOnStartup(void* dataPtr)
{
    HRenderer rdr = NULL;
    AppState* app = dataPtr;
    rnd_CreateRenderer(app->memory, app->options, &rdr);
	app->renderer = rdr;
#if 0
    vkxInitialize(0, app->options, NULL);
    vkxRequestQueues(1, (VkxQueueReq[])
    {
        {
            VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT,
            true,
            &gQueueFamily,
            &gCmdQueue
        }
    });
    vkxCreateDeviceAndSwapchain();
    vkxCreateCommandPool(gQueueFamily, &gCmdPool);
    vkxCreateCommandBuffer(gCmdPool, VK_COMMAND_BUFFER_LEVEL_PRIMARY, 0, &gCmdBuff);
    vkxCreateSemaphore(&gFrmBegin, 0);
    vkxCreateSemaphore(&gFrmEnd, 0);
	vkxCreateFence(&gDrawFence, 0);
    
    {
        uint32_t numSizes = 0;
        VkDescriptorPoolSize sizes[2];
        if (MAX_DESC_BUFFER > 0)
        {
            sizes[numSizes].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
            sizes[numSizes].descriptorCount = MAX_DESC_BUFFER;
            ++numSizes;
        }
        if (MAX_DESC_IMAGE > 0)
        {
            sizes[numSizes].type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
            sizes[numSizes].descriptorCount = MAX_DESC_IMAGE;
            ++numSizes;
        }
        VkDescriptorPoolCreateInfo info;
        info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        info.pNext = NULL;
        info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
        info.maxSets = MAX_DESC_SETS;
        info.poolSizeCount = numSizes;
        info.pPoolSizes = sizes;
        VK_ASSERT_Q(vkCreateDescriptorPool(gVkDev, &info, NULL, &app->descPool));
    }
    
    initGenerator(app, V2F(2.f, 2.f), 8, 8);
	gFence = gDrawFence[0];
#endif
}

static void renderFrame(AppState* app)
{
	rnd_RenderFrame(app->renderer);
	//VkFrame frame;
	//VkContext* vk = app->vulkan;
	//vk_BeginFrame(vk, &frame);
    //VkCommandBuffer cb = vk_GetCommandBuffer(frame,app->cRec);
    //vk_BeginCommandBufferOneOff(cb); //reset+begin
#if 0
    uint32_t imageIdx = 0;
    VkSemaphore frameBegin = gFrmBegin[gFrameIdx];
    VkSemaphore frameEnd = gFrmEnd[gFrameIdx];
    vkxAcquireNextImage(frameBegin, &imageIdx);
	VkImage displayImg = gDisplayImage[imageIdx];
    
    VkCommandBuffer cmdBuff = gCmdBuff[imageIdx];
    VkCommandBufferInfo cmdBuffInfo = { gQueueFamily, cmdBuff };
    vkWaitForFences(gVkDev, 1, &gFence, VK_TRUE, UINT64_MAX);
	gFence = gDrawFence[gFrameIdx];
	vkResetFences(gVkDev, 1, &gFence);
    
	vkResetCommandBuffer(cmdBuff, 0);
    vkxBeginCommandBufferOneOff(cmdBuff);
    
    vkCmdBindPipeline(cmdBuff, VK_PIPELINE_BIND_POINT_COMPUTE, app->generatorPipeline);
    vkCmdBindDescriptorSets(cmdBuff, VK_PIPELINE_BIND_POINT_COMPUTE, app->generatorPipelineLayout, 0, 1, &app->generatorDescr, 0, NULL);
	//buffer to writable state
	//vklBufferBarrierCSOutToVSIn
	//
    vkCmdPushConstants(cmdBuff, app->generatorPipelineLayout, VK_SHADER_STAGE_COMPUTE_BIT, 0, app->generatorSetupLen, app->generatorSetup);
    vkCmdDispatch(cmdBuff, 1, 1, 1);
	//buffer to VBO state
    
    vkxCmdClearColorImage(cmdBuffInfo, displayImg, VK_CLEAR_COLOR(0.f, 0.4f, 0.9f, 1.f));
    vkxCmdPreparePresent(cmdBuffInfo, displayImg);
    
	vkEndCommandBuffer(cmdBuff);
    VK_SUBMIT_INFO(submitInfo, cmdBuff, frameBegin, frameEnd);
	vkQueueSubmit(gCmdQueue, 1, &submitInfo, gFence);
    VK_PRESENT_INFO_KHR(presentInfo, imageIdx, frameEnd);
	vkQueuePresentKHR(gCmdQueue, &presentInfo);
    gFrameIdx = vkxNextFrame(gFrameIdx);
#endif
	//vk_EndFrame(vk, &frame);
}

void appOnShutdown(void* dataPtr)
{
	AppState* app = dataPtr;
    rnd_DestroyRenderer(app->renderer);
}

extern void appInitialize(HMemAlloc mem, const Options* opts, void* state);

extern bool appShouldKeepRunning(void);

int appMain(int argc, const char** argv)
{
	AppState appState = {0};
    appState.memory = memAllocCreate(MAX_FORWD, MAX_STACK, NULL, 0);
    appState.options = argParse(argc, argv, appState.memory);
	appInitialize(appState.memory, appState.options, &appState);
	while (appShouldKeepRunning())
        renderFrame(&appState);
	return 0;
}
