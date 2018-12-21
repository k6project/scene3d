#pragma once

static void CreateGraphicsContext(Renderer rnd, Options opts)
{
	VkQueueRequest queueRequest = { VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT, true };
	VkContextInfo contextInfo = { 0 };
	contextInfo.options = opts;
	contextInfo.numQueueReq = 1;
	contextInfo.queueReq = &queueRequest;
	vk_CreateContext(rnd->mem, &contextInfo, &rnd->vk);
}

static void DestroyGraphicsContext(Renderer rnd)
{
	vk_DeviceWaitIdle(rnd->vk);
	vk_DestroyDescriptorPool(rnd->vk, rnd->dPool);
	vk_DestroyRenderPass(rnd->vk, rnd->passes.forwardBase);
	vk_DestroyContext(rnd->vk);
}

static void CreateDescriptorPool(Renderer rnd)
{
	VkDescriptorPoolSize dpSize[] =
	{
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1 }
	};
	VkDescriptorPoolCreateInfo info = { 0 };
	info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
	info.maxSets = RND_DESCR_SET_MAX;
	info.poolSizeCount = ARRAY_LEN(dpSize);
	info.pPoolSizes = dpSize;
	vk_CreateDescriptorPool(rnd->vk, &info, &rnd->dPool);
}

static void CreateMemoryBuffers(Renderer rnd)
{
}
