#pragma once

#ifdef _MSC_VER
#define VK_USE_PLATFORM_WIN32_KHR
#else
#define VK_USE_PLATFORM_MACOS_MVK
#endif

#define VK_NO_PROTOTYPES
#include <vulkan/vulkan.h>

#define VK_MAX_BARRIERS_PER_CALL 4

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct
    {
        uint32_t family;
        uint32_t index;
        VkQueue queue;
    } VkQueueInfo;

    typedef struct VkContextImpl* VkContext;
    typedef struct VkDrawPassImpl* VkDrawPass;
    typedef struct VkTexture2DImpl* VkTexture2D;

    typedef struct
    {
        VkQueueFlags flags;
        bool present;
    } VkQueueRequest;

    typedef struct
    {
        VkContext parent;
        const struct Options* options;
        uint32_t numQueueReq;
        VkQueueRequest* queueReq;
    } VkRenderContextInfo;

    typedef struct
    {
        uint32_t index;
        uint32_t imgIdx;
        VkImage fbImage;
        VkSemaphore fbOk;
        VkSemaphore finished;
        VkFence fence;
        uint32_t numImgBarrier;
        VkImageMemoryBarrier imgBarrier[VK_MAX_BARRIERS_PER_CALL];
        uint32_t numBufBarrier;
        VkBufferMemoryBarrier bufBarrier[VK_MAX_BARRIERS_PER_CALL];
        uint32_t numMemBarrier;
        VkMemoryBarrier memBarrier[VK_MAX_BARRIERS_PER_CALL];
    } VkFrame;

    void vk_CreateRenderContext(HMemAlloc mem, const VkRenderContextInfo* info, VkContext* vkPtr);
    void vk_DeviceWaitIdle(VkContext vk);
    void vk_DestroyRenderContext(VkContext vk);
    void vk_BeginFrame(VkContext vk);
    VkFormat vk_GetSwapchainImageFormat(VkContext vk);
    VkCommandBuffer vk_GetPrimaryCommandBuffer(VkContext vk);
    void vk_BeginCommandBuffer(VkContext vk, VkCommandBuffer cb, const VkCommandBufferBeginInfo* info);
    void vk_EndCommandBuffer(VkContext vk, VkCommandBuffer cb);
    void vk_SubmitFrame(VkContext vk, uint32_t queue);
    void vk_CreateRenderPass(VkContext vk, const VkRenderPassCreateInfo* info, VkDrawPass* pass);
    void vk_SetClearColorValue(VkDrawPass pass, uint32_t att, Vec4f value);
    void vk_InitPassFramebuffer(VkContext vk, VkDrawPass pass, const VkTexture2D* textures);
    void vk_DestroyRenderPass(VkContext vk, VkDrawPass pass);
    void vk_CmdBeginRenderPass(VkContext vk, VkCommandBuffer cb, VkDrawPass pass);
    void vk_CmdEndRenderPass(VkContext vk, VkCommandBuffer cb);

#define vk_GetDisplayFormat(v) (v->surfFmt.format)

#define VKFN(c) TEST_Q( (c) == VK_SUCCESS ) 

#ifdef __cplusplus
}
#endif
