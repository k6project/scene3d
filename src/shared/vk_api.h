#pragma once

#include "global.h"

#include <stdbool.h>

#ifdef _MSC_VER
#define VK_USE_PLATFORM_WIN32_KHR
#define VK_LIBRARY L"vulkan-1.dll"
#else
#define VK_LIBRARY "@rpath/libvulkan.1.dylib"
#endif

#define VK_SWAPCHAIN_SIZE 3
#define VK_MAX_QUEUE_FAMILIES 8
#define VK_NO_PROTOTYPES
#include <vulkan/vulkan.h>

#ifdef __cplusplus
extern "C"
{
#endif

struct VkEnvironment
{
	void* library;
	VkInstance instance;
	VkSurfaceKHR surface;
	VkPhysicalDevice adapter;
	uint32_t numQueueFamilies;
	VkQueueFamilyProperties queueFamilies[VK_MAX_QUEUE_FAMILIES];
	VkSurfaceCapabilitiesKHR surfaceCaps;
};

typedef struct VkEnvironment VkEnvironment;
    
#define VULKAN_API_GOBAL(proc) extern PFN_vk ## proc vk ## proc;
#define VULKAN_API_INSTANCE(proc) extern PFN_vk ## proc vk ## proc;
#define VULKAN_API_DEVICE(proc) extern PFN_vk ## proc vk ## proc;
#include "vk_api.inl"

bool vkCreateAndInitInstanceAPP(void* dll, const
                                VkAllocationCallbacks* alloc,
                                VkInstance* inst);

bool vkCreateSurfaceAPP(VkInstance inst,
                        const VkAllocationCallbacks* alloc,
                        VkSurfaceKHR* surface);
    
bool vkGetAdapterAPP(VkInstance inst,
                     VkSurfaceKHR surface,
                     VkPhysicalDevice* adapter);

bool vkGetQueueFamiliesAPP(VkPhysicalDevice adapter,
                           uint32_t* count,
                           VkQueueFamilyProperties** props);
    
bool vkInitEnvironmentAPP(VkEnvironment* vkEnv, const VkAllocationCallbacks* alloc);

bool vkCreateAndInitDeviceAPP(VkPhysicalDevice adapter,
                              uint32_t numFamilies,
                              const uint32_t* queueFamilies,
                              const uint32_t* queueCounts,
                              const float* queuePriorities,
                              const VkAllocationCallbacks* alloc,
                              VkQueue* deviceQueues,
                              VkDevice* device);
    
#ifdef __cplusplus
}
#endif
