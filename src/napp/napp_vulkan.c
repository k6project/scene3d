#include "napp_vulkan.h"

#include <string.h>

#if defined(VK_USE_PLATFORM_WIN32_KHR)
#include <windows.h>
#else
#include <dlfcn.h>
#if defined(VK_USE_PLATFORM_MACOS_MVK)
#include <CoreFoundation/CoreFoundation.h>
#endif
#endif

static PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr;
#define VULKAN_API_GOBAL(f) static PFN_vk ## f vk ## f ;
#define VULKAN_API_INSTANCE(f) static PFN_vk ## f vk ## f ;
#define VULKAN_API_DEVICE(f) PFN_vk ## f vk ## f ;
#include "napp_vkinstance.h"
#include "napp_vkdevice.h"

static struct  
{
	void* DllHandle;
	VkInstance Instance;
	VkPhysicalDevice* PhysicalDevices;
} NAppVulkan;

static void NAppInitInstanceApi(VkInstance instance)
{
    if (instance != VK_NULL_HANDLE)
    {
#define VULKAN_API_GOBAL(proc) \
        TEST(vk ## proc = ( PFN_vk ## proc )vkGetInstanceProcAddr( instance, "vk" #proc ));
#include "napp_vkinstance.h"
    }
    else
    {
#define VULKAN_API_GOBAL(proc) \
        TEST(vk ## proc = ( PFN_vk ## proc )vkGetInstanceProcAddr( NULL, "vk" #proc ));
#include "napp_vkinstance.h"
    }
}

static void NAppInitDeviceApi(VkDevice device)
{
#define VULKAN_API_DEVICE(proc) \
    TEST(vk ## proc = ( PFN_vk ## proc )vkGetDeviceProcAddr( device, "vk" #proc ));
#include "napp_vkdevice.h"
}

void NAppVkInit()
{
	if (!NAppVulkan.DllHandle && !NAppVulkan.Instance)
	{
#if defined(VK_USE_PLATFORM_WIN32_KHR)
		NAppVulkan.DllHandle = LoadLibrary("vulkan-1.dll");
		vkGetInstanceProcAddr = (PFN_vkGetInstanceProcAddr)GetProcAddress(NAppVulkan.DllHandle, "vkGetInstanceProcAddr");
#elif defined(VK_USE_PLATFORM_MACOS_MVK)
        CFBundleRef bundle = CFBundleGetMainBundle();
        CFURLRef url = CFBundleCopyExecutableURL(bundle);
        CFStringRef path = CFURLCopyFileSystemPath(url, kCFURLPOSIXPathStyle);
        const char* pathStr = CFStringGetCStringPtr(path, kCFStringEncodingUTF8);
        const char* term = strrchr(pathStr, '/');
        static const char libName[] = "libvulkan.dylib";
        if (term)
        {
            int length = term - pathStr + 1;
            char* cwd = (char*)malloc(length + sizeof(libName));
            memcpy(cwd, pathStr, length);
            memcpy(cwd + length, libName, sizeof(libName));
            NAppVulkan.DllHandle = dlopen(cwd, RTLD_LOCAL | RTLD_NOW);
        }
        else
        {
            NAppVulkan.DllHandle = dlopen(libName, RTLD_LOCAL | RTLD_NOW);
        }
        CFRelease(path);
        CFRelease(url);
        CFRelease(bundle);
        vkGetInstanceProcAddr = (PFN_vkGetInstanceProcAddr)dlsym(NAppVulkan.DllHandle, "vkGetInstanceProcAddr");
#endif
        NAppInitInstanceApi(VK_NULL_HANDLE);
        //create instance
        VkApplicationInfo appInfo;
        VKINIT(appInfo, VK_STRUCTURE_TYPE_APPLICATION_INFO);
        //
        VkInstanceCreateInfo info;
        VKINIT(info, VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO);
        //
        VKTEST(vkCreateInstance(&info, NULL, &NAppVulkan.Instance));
        NAppInitInstanceApi(NAppVulkan.Instance);
	}
}

VkDevice NAppVkCreateDevice(int phDevice, const VkDeviceCreateInfo* info)
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
