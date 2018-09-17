#include "napp_vulkan.h"

static struct  
{
	VkInstance* Instance;
	VkPhysicalDevice* PhysicalDevices;
} NAppVulkan;

NAPP_API void NAppVulkanInit()
{
	//Load dll and create instance, load pointers to instance functions
}

NAPP_API VkDevice NAppVulkanCreateDevice(int physicalDevice, const VkDeviceCreateInfo* info, MGPUVulkanDevice* mgpuDevice)
{
	VkDevice device = VK_NULL_HANDLE;
	VkPhysicalDevice deviceHandle = NAppVulkan.PhysicalDevices[physicalDevice];
	if (vkCreateDevice(deviceHandle, info, NULL, &device) != VK_SUCCESS)
	{
		//vkAllocateMemory(hDevice, )
		device = VK_NULL_HANDLE;
	}
	//Todo load device functions (if mgpuDevice is not null, populate it, otherwise set default function pointers)
	return device;
}
