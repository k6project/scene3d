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
#define VK_MAX_QUEUES 8
#define VK_MAX_QUEUE_FAMILIES 8
#define VK_NO_PROTOTYPES
#include <vulkan/vulkan.h>

#ifdef __cplusplus
extern "C"
{
#endif
    
struct VkEnvironment;

typedef struct VkEnvironment* VkEnvironment;
    
#define VULKAN_API_GOBAL(proc) extern PFN_vk ## proc vk ## proc;
#define VULKAN_API_INSTANCE(proc) extern PFN_vk ## proc vk ## proc;
#define VULKAN_API_DEVICE(proc) extern PFN_vk ## proc vk ## proc;
#include "vk_api.inl"
    
/* Load library, create instance, choose adapter */
bool vkInitEnvironmentAPP(VkEnvironment* vkEnvPtr,
                          const VkAllocationCallbacks* alloc);

/* Request device queue to be created */
bool vkRequestQueueAPP(VkEnvironment vkEnv,
                       VkQueueFlags flags,
                       bool present);
    
/* Create device, get queues, create swapchain */
bool vkCreateDeviceAndSwapchainAPP(VkEnvironment vkEnv,
                                   const VkAllocationCallbacks* alloc,
                                   VkDevice* device,
                                   VkSwapchainKHR* swapchain,
                                   VkQueue** queues);

void vkDestroyEnvironmentAPP(VkEnvironment vkEnv,
                             const VkAllocationCallbacks* alloc);
    
#ifdef __cplusplus
}
#endif
