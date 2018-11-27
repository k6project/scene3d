#include "global.h"

#include "render.h"

#include "vk_api.h"

enum
{
	RND_QUEUE_DEFAULT,
    RND_NUM_QUEUES
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

#define RND_DESCR_SET_MAX 16u
#define RDR_CPU_MEM_TOTAL 65536u // 3/4 forward, 1/4 stack
#define RDR_CPU_MEM_FORWD ((RDR_CPU_MEM_TOTAL >> 1) + (RDR_CPU_MEM_TOTAL >> 2))
#define RDR_CPU_MEM_STACK (RDR_CPU_MEM_TOTAL - RDR_CPU_MEM_FORWD)

struct RendererImpl
{
	MemAlloc mem;
	VkContext vk;
    VkDescriptorPool dPool;
	VkDrawPass basePass;
};

/*
    For procedurally generated mesh we create unified set of descriptors with bindings for different stages
*/

static void rnd_CreateDescriptorPool(Renderer rnd);
static void rnd_CreateRenderPasses(Renderer rnd);

void rnd_CreateRenderer(MemAlloc mem, const struct Options* opts, Renderer* rndPtr)
{
    size_t memBytes = memSubAllocSize(RDR_CPU_MEM_TOTAL);
    void* parentMem = memForwdAlloc(mem, memBytes);
    MemAlloc local = memAllocCreate(RDR_CPU_MEM_FORWD, RDR_CPU_MEM_STACK, parentMem, memBytes);
    Renderer rnd = memForwdAlloc(local, sizeof(struct RendererImpl));
    rnd->mem = local;
    VkQueueRequest queueRequest = {VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT, true};
    VkContextInfo contextInfo = {0};
    contextInfo.options = opts;
    contextInfo.numQueueReq = 1;
    contextInfo.queueReq = &queueRequest;
    vk_CreateContext(mem, &contextInfo, &rnd->vk);
    rnd_CreateDescriptorPool(rnd);
    rnd_CreateRenderPasses(rnd);
    *rndPtr = rnd;
}

void rnd_DestroyRenderer(Renderer rnd)
{
	vk_DeviceWaitIdle(rnd->vk);
    vk_DestroyDescriptorPool(rnd->vk, rnd->dPool);
    vk_DestroyRenderPass(rnd->vk, rnd->basePass);
    vk_DestroyContext(rnd->vk);
}

void rnd_RenderFrame(Renderer rnd)
{
	vk_BeginFrame(rnd->vk);
    //every pcg element has a secondary command buffer of its own
    //if regeneration is needed, it runs before drawing
	VkCommandBuffer pcb = vk_GetPrimaryCommandBuffer(rnd->vk);
    //dispatch secondary command buffer for content generation
	vk_CmdBeginRenderPass(rnd->vk, pcb, rnd->basePass);
	vk_CmdEndRenderPass(rnd->vk, pcb);
	vk_SubmitFrame(rnd->vk, RND_QUEUE_DEFAULT);
}

void rnd_CreateDescriptorPool(Renderer rnd)
{
    VkDescriptorPoolSize dpSize[] =
    {
        {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1}
    };
    VkDescriptorPoolCreateInfo info = { 0 };
    info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    info.maxSets = RND_DESCR_SET_MAX;
    info.poolSizeCount = ARRAY_LEN(dpSize);
    info.pPoolSizes = dpSize;
    vk_CreateDescriptorPool(rnd->vk, &info, &rnd->dPool);
}

void rnd_CreateRenderPasses(Renderer rnd)
{
    memStackFramePush(rnd->mem);
    VkAttachmentDescription attachments[BASE_PASS_NUM_ATTACHMENTS] = { 0 };
    {
        VkAttachmentDescription* att = &attachments[BASE_PASS_ATTACHMENT_COLOR0];
        att->format = vk_GetSwapchainImageFormat(rnd->vk);
        att->samples = VK_SAMPLE_COUNT_1_BIT;
        att->loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        att->storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        att->stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        att->stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        att->initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        att->finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    }
    VkSubpassDescription subpasses[BASE_PASS_NUM_SUBPASSES] = { 0 };
    {
        uint32_t attRefs = 1;
        VkAttachmentReference* refs = memStackAlloc(rnd->mem, attRefs * sizeof(VkAttachmentReference));
        refs[0].attachment = BASE_PASS_ATTACHMENT_COLOR0;
        refs[0].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        VkSubpassDescription* spass = &subpasses[BASE_PASS_SUBPASS_MAIN];
        spass->pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        spass->colorAttachmentCount = attRefs;
        spass->pColorAttachments = refs;
    }
    {
        VkSubpassDependency dep = { 0 };
        dep.srcSubpass = VK_SUBPASS_EXTERNAL;
        dep.dstSubpass = 0;
        dep.srcStageMask = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
        dep.dstStageMask = VK_PIPELINE_STAGE_VERTEX_INPUT_BIT;
        dep.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT | VK_ACCESS_SHADER_READ_BIT;
        dep.dstAccessMask = VK_ACCESS_INDEX_READ_BIT | VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;
    }
    VkRenderPassCreateInfo passInfo = { 0 };
    passInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    passInfo.attachmentCount = BASE_PASS_NUM_ATTACHMENTS;
    passInfo.pAttachments = attachments;
    passInfo.subpassCount = BASE_PASS_NUM_SUBPASSES;
    passInfo.pSubpasses = subpasses;
    vk_CreateRenderPass(rnd->vk, &passInfo, &rnd->basePass);
    vk_SetClearColorValue(rnd->basePass, 0, V4F(0.f, 0.4f, 0.9f, 1.f));
    vk_InitPassFramebuffer(rnd->vk, rnd->basePass, NULL);
    memStackFramePop(rnd->mem);
}
