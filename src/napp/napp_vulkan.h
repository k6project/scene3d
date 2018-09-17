#pragma once

#include "napp.h"

#define VK_NO_PROTOTYPES
#include <vulkan/vulkan.h>

//declare pointers to API functions (primary device)

//declare structure for MGPUVulkanDevice ()

typedef struct MGPUVulkanDevice
{
	PFN_vkAllocateMemory AllocateMemory;
} MGPUVulkanDevice;

NAPP_API void NAppVulkanInit();

NAPP_API VkDevice NAppVulkanCreateDevice(int physicalDevice, const VkDeviceCreateInfo* info, MGPUVulkanDevice* mgpuDevice);
