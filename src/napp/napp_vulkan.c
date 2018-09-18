#include "napp_vulkan.h"

static PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr;
#define VULKAN_API_GOBAL(f) static PFN_vk ## f vk ## f ;
#define VULKAN_API_INSTANCE(f) static PFN_vk ## f vk ## f ;
#define VULKAN_API_DEVICE(f) PFN_vk ## f ## vk ## f ;
#include "napp_vkinstance.h"
#include "napp_vkdevice.h"

static struct  
{
	void* DllHandle;
	VkInstance* Instance;
	VkPhysicalDevice* PhysicalDevices;
} NAppVulkan;

static void NAppInitGlobalApi();
static void NAppInitInstanceApi(VkInstance instance);
static void NAppInitDeviceApi(VkDevice device);

NAPP_API void NAppVkInit()
{
	if (!NAppVulkan.DllHandle && !NAppVulkan.Instance)
	{
#if defined(VK_USE_PLATFORM_WIN32_KHR)
		NAppVulkan.DllHandle = LoadLibrary("vulkan-1.dll");
		vkGetInstanceProcAddr = (PFN_vkGetInstanceProcAddr)GetProcAddress(NAppVulkan.DllHandle, "vkGetInstanceProcAddr");
#else
#endif
	}
}

NAPP_API VkDevice NAppVkCreateDevice(int phDevice, const VkDeviceCreateInfo* info)
{
	VkDevice device = VK_NULL_HANDLE;
	VkPhysicalDevice deviceHandle = NAppVulkan.PhysicalDevices[phDevice];
	if (vkCreateDevice(deviceHandle, info, NULL, &device) == VK_SUCCESS)
	{
		NAppInitDeviceApi(device);
		//
	}
	return device;
}

void NAppInitGlobalApi()
{
}

void NAppInitInstanceApi(VkInstance instance)
{
}

void NAppInitDeviceApi(VkDevice device)
{
}
