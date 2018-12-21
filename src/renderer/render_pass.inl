#pragma once

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

static void CreateRenderPasses(Renderer rnd)
{
	mem_StackFramePush(rnd->mem);
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
		VkAttachmentReference* refs = mem_StackAlloc(rnd->mem, attRefs * sizeof(VkAttachmentReference));
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
	vk_CreateRenderPass(rnd->vk, &passInfo, &rnd->passes.forwardBase);
	vk_SetClearColorValue(rnd->passes.forwardBase, 0, (float[]) { 0.f, 0.4f, 0.9f, 1.f });
	vk_InitPassFramebuffer(rnd->vk, rnd->passes.forwardBase, NULL);
	mem_StackFramePop(rnd->mem);
}
