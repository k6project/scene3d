#include "vk_api.h"

#include "args.h"

#include <stdlib.h>
#include <string.h>

static PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr;
#define VULKAN_API_GOBAL(proc) PFN_vk ## proc vk ## proc = NULL;
#define VULKAN_API_INSTANCE(proc) PFN_vk ## proc vk ## proc = NULL;
#define VULKAN_API_DEVICE(proc) PFN_vk ## proc vk ## proc = NULL;
#include "vk_api.inl"

static const char* VK_REQUIRED_LAYERS[] =
{
	"VK_LAYER_LUNARG_standard_validation"
};

static const unsigned int VK_NUM_REQUIRED_LAYERS = sizeof(VK_REQUIRED_LAYERS) / sizeof(const char*);

static const char* VK_REQUIRED_EXTENSIONS[] =
{
	VK_KHR_SURFACE_EXTENSION_NAME
#if defined(VK_USE_PLATFORM_MACOS_MVK)
	, VK_MVK_MACOS_SURFACE_EXTENSION_NAME
#elif defined(VK_USE_PLATFORM_WIN32_KHR)
	, VK_KHR_WIN32_SURFACE_EXTENSION_NAME
#endif
#ifdef _DEBUG
	, VK_EXT_DEBUG_REPORT_EXTENSION_NAME
#endif
};

static const unsigned int VK_NUM_REQUIRED_EXTENSIONS = sizeof(VK_REQUIRED_EXTENSIONS) / sizeof(const char*);

bool vkCreateAndInitInstanceAPP(void* dll, const VkAllocationCallbacks* alloc, VkInstance* inst)
{
    vkGetInstanceProcAddr = appGetLibraryProc(dll, "vkGetInstanceProcAddr");
    VERIFY(vkGetInstanceProcAddr, false, "ERROR: Failed to get pointer to vkGetInstanceProcAddr");
#define VULKAN_API_GOBAL(proc) \
    vk ## proc = ( PFN_vk ## proc )vkGetInstanceProcAddr( NULL, "vk" #proc ); \
    VERIFY(vk ## proc, false, "ERROR: Failed to get pointer to vk" #proc );
#include "vk_api.inl"
    appPrintf("Loaded global function pointers\n");
    
	VkApplicationInfo appInfo;
	static char appName[APP_NAME_MAX];
	appTCharToUTF8(appName, gOptions->appName, APP_NAME_MAX);
	VK_INIT(appInfo, VK_STRUCTURE_TYPE_APPLICATION_INFO);
	appInfo.pApplicationName = appName;
	appInfo.apiVersion = VK_MAKE_VERSION(1, 0, 0);
	VkInstanceCreateInfo createInfo;
	VK_INIT(createInfo, VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO);
	createInfo.pApplicationInfo = &appInfo;
	
    createInfo.enabledLayerCount = VK_NUM_REQUIRED_LAYERS + gOptions->numLayers;
	if (createInfo.enabledLayerCount > VK_NUM_REQUIRED_LAYERS)
	{
		char** layers = (char**)malloc(createInfo.enabledLayerCount * sizeof(const char*));
		memcpy(layers, VK_REQUIRED_LAYERS, VK_NUM_REQUIRED_LAYERS * sizeof(const char*));
		memcpy(layers + VK_NUM_REQUIRED_LAYERS, gOptions->layers, gOptions->numLayers * sizeof(const char*));
		createInfo.ppEnabledLayerNames = (const char**)layers;
	}
	else
	{
		createInfo.ppEnabledLayerNames = VK_REQUIRED_LAYERS;
	}
    appPrintf("Instance debug layers:\n");
    for (unsigned int i = 0; i < createInfo.enabledLayerCount; i++)
    {
        appPrintf("  %s\n", createInfo.ppEnabledLayerNames[i]);
    }
    
	createInfo.enabledExtensionCount = VK_NUM_REQUIRED_EXTENSIONS + gOptions->numExtensions;
	if (createInfo.enabledExtensionCount > VK_NUM_REQUIRED_EXTENSIONS)
	{
		char** ext = (char**)malloc(createInfo.enabledExtensionCount * sizeof(const char*));
		memcpy(ext, VK_REQUIRED_EXTENSIONS, VK_NUM_REQUIRED_EXTENSIONS * sizeof(const char*));
		memcpy(ext + VK_NUM_REQUIRED_EXTENSIONS, gOptions->extensions, gOptions->numExtensions * sizeof(const char*));
		createInfo.ppEnabledExtensionNames = (const char**)ext;
	}
	else
	{
		createInfo.ppEnabledExtensionNames = VK_REQUIRED_EXTENSIONS;
	}
    appPrintf("Instance extensions:\n");
    for (unsigned int i = 0; i < createInfo.enabledExtensionCount; i++)
    {
        appPrintf("  %s\n", createInfo.ppEnabledExtensionNames[i]);
    }

    VkResult result = vkCreateInstance(&createInfo, alloc, inst);
	if (createInfo.enabledExtensionCount > VK_NUM_REQUIRED_EXTENSIONS)
		free((void*)createInfo.ppEnabledExtensionNames);
	if (createInfo.enabledLayerCount > VK_NUM_REQUIRED_LAYERS)
		free((void*)createInfo.ppEnabledLayerNames);
    
    if (result == VK_SUCCESS)
    {
#define VULKAN_API_INSTANCE(proc) \
        vk ## proc = ( PFN_vk ## proc )vkGetInstanceProcAddr( *inst, "vk" #proc ); \
        VERIFY(vk ## proc, false, "ERROR: Failed to get pointer to vk" #proc );
#include "vk_api.inl"
        appPrintf("Loaded instance-specific function pointers\n");
    }
    else
    {
        appPrintf("ERROR: Failed to create Vulkan instance (%d)\n", result);
    }
	return true;
}

bool vkGetAdapterAPP(VkInstance inst, VkSurfaceKHR surface, VkPhysicalDevice* adapter)
{
    *adapter = VK_NULL_HANDLE;
    unsigned int num = 0, idx = 0xff;
    if (vkEnumeratePhysicalDevices(inst, &num, NULL) == VK_SUCCESS && num)
    {
        VkPhysicalDevice* adapters = malloc(sizeof(VkPhysicalDevice) * num);
        if (vkEnumeratePhysicalDevices(inst, &num, adapters) == VK_SUCCESS)
        {
            for (unsigned int i = 0; i < num; i++)
            {
                unsigned int numFamilies = 0;
                VkPhysicalDeviceProperties props;
                vkGetPhysicalDeviceProperties(adapters[i], &props);
                vkGetPhysicalDeviceQueueFamilyProperties(adapters[i], &numFamilies, 0);
                appPrintf("%u: %s (%u queue fam.)\n", i, props.deviceName, numFamilies);
                if (*adapter == VK_NULL_HANDLE && props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
                {
                    VkBool32 canPresent = VK_FALSE;
                    for (unsigned int j = 0; j < numFamilies; j++)
                    {
                        vkGetPhysicalDeviceSurfaceSupportKHR(adapters[i], j, surface, &canPresent);
                        if (canPresent == VK_TRUE)
                        {
                            *adapter = adapters[i];
                            idx = i;
                            break;
                        }
                    }
                }
            }
        }
        free(adapters);
    }
    if (*adapter != VK_NULL_HANDLE)
    {
        appPrintf("Unsing adapter %u\n", idx);
        return true;
    }
    return false;
}
