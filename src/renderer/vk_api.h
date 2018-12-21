#pragma once

#ifdef _MSC_VER
#define VK_USE_PLATFORM_WIN32_KHR
#else
#define VK_USE_PLATFORM_MACOS_MVK
#endif

#define VK_NO_PROTOTYPES
#include <vulkan/vulkan.h>

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct VkContextImpl* VkContext;
typedef struct VkTexture2DImpl* VkTexture2D;
typedef struct VkRenderPassDataImpl* VkRenderPassData;

typedef struct
{
    VkQueueFlags flags;
    bool present;
} VkQueueRequest;

typedef struct
{
    VkContext parent;
    Options options;
    uint32_t numQueueReq;
    VkQueueRequest* queueReq;
} VkContextInfo;

void vk_CreateContext(MemAlloc mem, const VkContextInfo* info, VkContext* vkPtr);
void vk_DeviceWaitIdle(VkContext vk);
void vk_DestroyContext(VkContext vk);

void vk_BeginFrame(VkContext vk);
void vk_BeginCommandBuffer(VkContext vk, VkCommandBuffer cb, const VkCommandBufferBeginInfo* info);
void vk_EndCommandBuffer(VkContext vk, VkCommandBuffer cb);
void vk_SubmitFrame(VkContext vk, uint32_t queue);

void vk_CreateDescriptorPool(VkContext vk, const VkDescriptorPoolCreateInfo* info, VkDescriptorPool* pool);
void vk_DestroyDescriptorPool(VkContext, VkDescriptorPool pool);

void vk_MallocBuffer(VkContext vk, VkBuffer buff, VkMemoryPropertyFlags flags);

void vk_CreateRenderPass(VkContext vk, const VkRenderPassCreateInfo* info, VkRenderPassData* pass);
void vk_SetClearColorValue(VkRenderPassData pass, uint32_t att, float value[4]);
void vk_InitPassFramebuffer(VkContext vk, VkRenderPassData pass, const VkTexture2D* textures);
void vk_DestroyRenderPass(VkContext vk, VkRenderPassData pass);

void vk_CmdBeginRenderPass(VkContext vk, VkCommandBuffer cb, VkRenderPassData pass);
void vk_CmdEndRenderPass(VkContext vk, VkCommandBuffer cb); 

VkFormat vk_GetSwapchainImageFormat(VkContext vk);
VkCommandBuffer vk_GetPrimaryCommandBuffer(VkContext vk);

#ifdef __cplusplus
}
#endif
