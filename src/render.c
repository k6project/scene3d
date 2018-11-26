#include "global.h"

#include "render.h"

enum
{
	RND_QUEUE_DEFAULT
};

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

struct RendererImpl
{
	HMemAlloc mem;
	HVkContext vk;
	HVkRenderPass basePass;
};

void rnd_CreateRenderer(HMemAlloc mem, const struct Options* opts, HRenderer* rndPtr)
{
    size_t memBytes = memSubAllocSize(RDR_CPU_MEM_TOTAL);
    void* parentMem = memForwdAlloc(mem, memBytes);
    HMemAlloc local = memAllocCreate(RDR_CPU_MEM_FORWD, RDR_CPU_MEM_STACK, parentMem, memBytes);
    HRenderer rnd = memForwdAlloc(local, sizeof(struct RendererImpl));
    rnd->mem = local;
    VkQueueRequest queueRequest = {VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT, true};
    VkRenderContextInfo contextInfo = {0};
    contextInfo.options = opts;
    contextInfo.numQueueReq = 1;
    contextInfo.queueReq = &queueRequest;
    HVkContext vk = NULL;
    vk_CreateRenderContext(mem, &contextInfo, &vk);
    VkAttachmentDescription attachments[BASE_PASS_NUM_ATTACHMENTS] = {0};
    {
        VkAttachmentDescription* att = &attachments[BASE_PASS_ATTACHMENT_COLOR0];
		att->format = vk_GetSwapchainImageFormat(vk);
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
    vk_CreateRenderPass(vk, &passInfo, &rnd->basePass);
    vk_InitPassFramebuffer(vk, rnd->basePass, NULL);
    rnd->vk = vk;
    *rndPtr = rnd;
}

void rnd_DestroyRenderer(HRenderer rnd)
{
    vk_DestroyRenderPass(rnd->vk, rnd->basePass);
    vk_DestroyRenderContext(rnd->vk);
}

void rnd_RenderFrame(HRenderer rnd)
{
	vk_BeginFrame(rnd->vk);
	VkCommandBuffer pcb = vk_GetPrimaryCommandBuffer(rnd->vk);
	vk_CmdBeginRenderPass(rnd->vk, pcb, rnd->basePass);
	vk_CmdEndRenderPass(rnd->vk, pcb);
	vk_SubmitFrame(rnd->vk, RND_QUEUE_DEFAULT);
}
