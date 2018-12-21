#include "../global.h"
#include "../args.h"

#include "vk_api.h"
#include "renderer.h"

#define RND_VBO_VECTORS   2u
#define RND_DESCR_SET_MAX 16u
#define RDR_CPU_MEM_TOTAL 65536u // 3/4 forward, 1/4 stack
#define RDR_CPU_MEM_FORWD ((RDR_CPU_MEM_TOTAL >> 1) + (RDR_CPU_MEM_TOTAL >> 2))
#define RDR_CPU_MEM_STACK (RDR_CPU_MEM_TOTAL - RDR_CPU_MEM_FORWD)

struct RendererImpl
{
	MemAlloc mem;
	VkContext vk;
	VkDescriptorPool dPool;
	struct
	{
		VkRenderPassData forwardBase;
	} passes;
};

struct RenderStateImpl
{
	uint32_t count;
};

#include "render_init.inl"
#include "render_pass.inl"

Renderer rnd_CreateRenderer(MemAlloc mem, Options opts)
{
	size_t memBytes = mem_SubAllocSize(RDR_CPU_MEM_TOTAL);
	void* parentMem = mem_ForwdAlloc(mem, memBytes);
	MemAlloc local = mem_AllocCreate(RDR_CPU_MEM_FORWD, RDR_CPU_MEM_STACK, parentMem, memBytes);
	Renderer rnd = mem_ForwdAlloc(local, sizeof(struct RendererImpl));
	rnd->mem = local;
	CreateGraphicsContext(rnd, opts);
	CreateDescriptorPool(rnd);
	CreateMemoryBuffers(rnd);
	CreateRenderPasses(rnd);
	return rnd;
}

void rnd_DestroyRenderer(Renderer rnd)
{
	DestroyGraphicsContext(rnd);
}

void rnd_RenderFrame(Renderer rnd)
{
	mem_StackFramePush(rnd->mem);

	vk_BeginFrame(rnd->vk);
	//every pcg element has a secondary command buffer of its own
	//if regeneration is needed, it runs before drawing
	VkCommandBuffer pcb = vk_GetPrimaryCommandBuffer(rnd->vk);
	//dispatch secondary command buffer for content generation
	vk_CmdBeginRenderPass(rnd->vk, pcb, rnd->passes.forwardBase);
	vk_CmdEndRenderPass(rnd->vk, pcb);
	vk_SubmitFrame(rnd->vk, 0);

	//RenderState* subset = scn_GetSubsetToRender(scn, rnd->mem); //null-terminated list of render states to consider
	//ExecuteComputePass(rnd, rnd->passes.pcGenerate, subset); // generate any procedural assets that needs to be re-generated
	//ExecuteRenderPass(rnd, rnd->passes.forwardBase, subset); // draw all render states
	mem_StackFramePop(rnd->mem);
}
