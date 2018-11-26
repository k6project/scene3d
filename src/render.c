#include "global.h"

#include "render.h"

enum
{
    BASE_PASS_ATTACHMENT_COLOR0,
    BASE_PASS_NUM_ATTACHMENTS
};

enum
{
    BASE_PASS_SUBPASS_MAIN,
    BASE_PASS_NUM_SUBPASSES
};

#define RDR_CPU_MEM_TOTAL 65536u // 3/4 forward, 1/4 stack
#define RDR_CPU_MEM_FORWD ((RDR_CPU_MEM_TOTAL >> 1) + (RDR_CPU_MEM_TOTAL >> 2))
#define RDR_CPU_MEM_STACK (RDR_CPU_MEM_TOTAL - RDR_CPU_MEM_FORWD)

void rdr_CreateRenderer(HMemAlloc mem, const struct Options* opts, Renderer** rdrPtr)
{
    size_t memBytes = memSubAllocSize(RDR_CPU_MEM_TOTAL);
    void* parentMem = memForwdAlloc(mem, memBytes);
    HMemAlloc local = memAllocCreate(RDR_CPU_MEM_FORWD, RDR_CPU_MEM_STACK, parentMem, memBytes);
    Renderer* rdr = memForwdAlloc(local, sizeof(Renderer));
    rdr->mem = local;
    rdr->outer = mem;
    //TODO
    
    VkQueueRequest queueRequest = {VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT, true};
    VkContextInfo contextInfo = {0};
    contextInfo.options = opts;
    contextInfo.numQueueReq = 1;
    contextInfo.queueReq = &queueRequest;
    VkContext* vk = NULL;
    vk_CreateContext(vk, &contextInfo, mem);
    vk_InitCommandRecorder(vk, &rdr->cmdRec, 0);
    
    VkAttachmentDescription attachments[BASE_PASS_NUM_ATTACHMENTS] = {0};
    {
        VkAttachmentDescription* att = &attachments[BASE_PASS_ATTACHMENT_COLOR0];
        att->format = vk_GetDisplayFormat(vk);
        att->samples = VK_SAMPLE_COUNT_1_BIT;
        att->loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        att->storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        att->stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        att->stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        att->initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        att->finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    }

    VkSubpassDescription subpasses[BASE_PASS_NUM_SUBPASSES] = {0};
    {
        VkAttachmentReference aRefs[] =
        {
            {BASE_PASS_ATTACHMENT_COLOR0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL}
        };
        VkSubpassDescription* spass = &subpasses[BASE_PASS_SUBPASS_MAIN];
        spass->pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        spass->colorAttachmentCount = ARRAY_LEN(aRefs);
        spass->pColorAttachments = aRefs;
    }
    
    VkRenderPassCreateInfo passInfo = {0};
    passInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    passInfo.attachmentCount = BASE_PASS_NUM_ATTACHMENTS;
    passInfo.pAttachments = attachments;
    passInfo.subpassCount = BASE_PASS_NUM_SUBPASSES;
    passInfo.pSubpasses = subpasses;
    vk_CreateRenderPass(vk, &passInfo, &rdr->basePass);
    vk_InitPassFramebuffer(vk, rdr->basePass, NULL);
    rdr->vulkan = vk;
    *rdrPtr = rdr;
}

void rdr_DestroyRenderer(Renderer** rdrPtr)
{
    Renderer* rdr = *rdrPtr;
    VkContext* vk = rdr->vulkan;
    //TODO
    vk_DestroyRenderPass(vk, &rdr->basePass);
    vk_DestroyCommandRecorder(vk, &rdr->cmdRec);
    vk_DestroyContext(rdr->vulkan);
    *rdrPtr = NULL;
}
