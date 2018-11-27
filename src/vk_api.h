#pragma once

#define VK_SHADER_MAIN "main"

#ifdef _MSC_VER
#define VK_USE_PLATFORM_WIN32_KHR
#define VK_LIBRARY "vulkan-1.dll"
#else
#define VK_USE_PLATFORM_MACOS_MVK
#define VK_LIBRARY "@rpath/libvulkan.1.dylib"
#endif

#define VK_NO_PROTOTYPES
#include <vulkan/vulkan.h>

#define VK_ASSERT_Q(c) \
    ASSERT_Q(c == VK_SUCCESS)

#define VK_ASSERT(c, m, ...) \
    ASSERT(c == VK_SUCCESS, m, __VA_ARGS__)

#ifdef __cplusplus
extern "C"
{
#endif
 
#define VULKAN_API_GOBAL(proc) extern PFN_vk ## proc vk ## proc;
#define VULKAN_API_INSTANCE(proc) extern PFN_vk ## proc vk ## proc;
#define VULKAN_API_DEVICE(proc) extern PFN_vk ## proc vk ## proc;
#include "vk_api.inl"

///////////////////////////////////////

#include "vk_types.h"
    
struct Options;

typedef struct
{
	VkQueueFlags flags;
	bool present;
} VklQueueReq;

typedef struct
{
	const struct Options* appOpts;
	uint32_t numQueueReq;
	VklQueueReq* queueReq;
} VklOptions;

void vklInitialize(HVulkan* vkPtr, const VklOptions* opts, HMemAlloc memory, HVkQueue** pQueues);
void vklFinalize(HVulkan vk);

///////////////////////////////////////
    
typedef struct
{
    VkQueueFlags flags;
    bool present; 
    uint32_t* outFamily;
    VkQueue* outQueue;
} VkxQueueReq;
    
typedef struct
{
    uint32_t queueFamily;
    VkCommandBuffer commandBuffer;
} VkCommandBufferInfo;

extern const VkAllocationCallbacks* gVkAlloc;
extern VkDevice gVkDev;
extern VkSwapchainKHR gVkSwapchain;
extern VkSurfaceFormatKHR gSurfaceFormat;
extern VkImage* gDisplayImage;

void vkxInitialize(size_t maxMem, const struct Options* opts, const VkAllocationCallbacks* alloc);
void vkxFinalize(void); 
void vkxRequestQueues(uint32_t count, VkxQueueReq* request);
void vkxCreateDeviceAndSwapchain(void);
void vkxCreateCommandPool(uint32_t queueFamily, VkCommandPool* pool);
void vkxCreateCommandBuffer(VkCommandPool pool, VkCommandBufferLevel level, uint32_t count, VkCommandBuffer** cbuff);
void vkxDestroyCommandBuffer(VkCommandPool pool, uint32_t count, VkCommandBuffer* ptr);
void vkxCreateSemaphore(VkSemaphore** out, uint32_t count);
void vkxDestroySemaphore(VkSemaphore* sem, uint32_t count);
void vkxCreateFence(VkFence** out, uint32_t count);
void vkxDestroyFence(VkFence* fen, uint32_t count);
void vkxCmdClearColorImage(VkCommandBufferInfo info, VkImage img, VkClearColorValue* color);
void vkxAcquireNextImage(VkSemaphore sem, uint32_t* image);
void vkxCmdPreparePresent(VkCommandBufferInfo info, VkImage img);
uint32_t vkxNextFrame(uint32_t current);
VkDeviceMemory vkxMallocBuffer(VkBuffer buff, VkMemoryPropertyFlags flags);
    
#define vkxBeginCommandBufferOneOff(cb) \
    do { VkCommandBufferBeginInfo beginInfo = { \
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO, .pNext = NULL, \
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, .pInheritanceInfo = NULL \
    };  VK_ASSERT_Q(vkBeginCommandBuffer(cb, &beginInfo)); } while(0)

#define VK_SUBMIT_INFO(n, cb, ws, ss) \
    VkPipelineStageFlags _##n##_wFlags_ = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;\
    VkSubmitInfo n = { .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO, .pNext = NULL, \
        .waitSemaphoreCount = (ws) ? 1 : 0, .pWaitSemaphores = (ws) ? &ws : NULL, \
        .pWaitDstStageMask = &_##n##_wFlags_, .commandBufferCount = 1, .pCommandBuffers = &cb,\
        .signalSemaphoreCount = (ss) ? 1 : 0, .pSignalSemaphores = (ss) ? &ss : NULL\
    }
    
#define VK_PRESENT_INFO_KHR(n, i, ws) \
    VkPresentInfoKHR n = { .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR, .pNext = NULL, \
        .waitSemaphoreCount = (ws) ? 1 : 0, .pWaitSemaphores = (ws) ? &ws : NULL, \
        .swapchainCount = 1, .pSwapchains = &gVkSwapchain, .pImageIndices = &i, .pResults = NULL \
    }
    
#define VK_CLEAR_COLOR(r, g, b, a) ((VkClearColorValue*) (Color) { r, g, b, a })

#ifdef __cplusplus
}
#endif
