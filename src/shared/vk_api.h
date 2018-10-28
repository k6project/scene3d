#pragma once

#include "global.h"

#include <stdbool.h>

#ifdef _MSC_VER
#define VK_USE_PLATFORM_WIN32_KHR
#define VK_LIBRARY L"vulkan-1.dll"
#else
#define VK_USE_PLATFORM_MACOS_MVK
#define VK_LIBRARY "@rpath/libvulkan.1.dylib"
#endif

#define VK_SWAPCHAIN_SIZE 3
#define VK_NO_PROTOTYPES
#include <vulkan/vulkan.h>

#define VK_ASSERT(c, m) \
    ASSERT(c == VK_SUCCESS, m)

#define VK_QUEUE_GCT \
    (VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT)

#ifdef __cplusplus
extern "C"
{
#endif
 
#define VULKAN_API_GOBAL(proc) extern PFN_vk ## proc vk ## proc;
#define VULKAN_API_INSTANCE(proc) extern PFN_vk ## proc vk ## proc;
#define VULKAN_API_DEVICE(proc) extern PFN_vk ## proc vk ## proc;
#include "vk_api.inl"
    
typedef struct
{
    VkQueueFlags flags;
    bool present; 
    uint32_t* outFamily;
    VkQueue* outQueue;
} VkQueueRequest;

extern const VkAllocationCallbacks* gVkAlloc;
extern VkDevice gVkDev;
extern VkSwapchainKHR gVkSwapchain;
extern VkSurfaceFormatKHR gSurfaceFormat;

void vkInitializeAPP(size_t maxMem, const VkAllocationCallbacks* alloc);
void vkFinalizeAPP(void); 
void vkRequestQueuesAPP(uint32_t count, VkQueueRequest* request);
void vkCreateDeviceAndSwapchainAPP();
void vkCreateCommandBufferAPP(VkCommandBufferAllocateInfo* info, VkCommandBuffer** out);
void vkDestroyCommandBufferAPP(VkCommandPool pool, uint32_t count, VkCommandBuffer* ptr);

#define VK_CMDPOOL_CREATE_INFO(n,qf) \
    VkCommandPoolCreateInfo n = { \
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO, \
        .pNext = NULL , .flags = 0, .queueFamilyIndex = qf }
   
#define VK_CMDBUFF_CREATE_INFO(n,cp,l,num) \
    VkCommandBufferAllocateInfo n = { \
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO, \
        .pNext = NULL, .commandPool = cp ,.level = l , .commandBufferCount = num }

#ifdef __cplusplus
}
#endif
