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

static const uint32_t VK_NUM_REQUIRED_LAYERS = sizeof(VK_REQUIRED_LAYERS) / sizeof(const char*);

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

struct VkQueueReq
{
    uint32_t family : 16;
    uint32_t index  : 16;
};

struct VkEnvironment
{
    void* library;
    VkInstance instance;
    VkSurfaceKHR surface;
    VkPhysicalDevice adapter;
    uint32_t numQueueFamilies;
    uint32_t numQueuesRequested;
    struct VkQueueReq queueRequests[VK_MAX_QUEUES];
    uint8_t queueCount[VK_MAX_QUEUE_FAMILIES];
    VkQueueFamilyProperties queueFamilies[VK_MAX_QUEUE_FAMILIES];
    VkSurfaceCapabilitiesKHR surfaceCaps;
    uint32_t numPresentModes;
    VkPresentModeKHR presentModes[VK_PRESENT_MODE_RANGE_SIZE_KHR];
};

static const uint32_t VK_NUM_REQUIRED_EXTENSIONS = sizeof(VK_REQUIRED_EXTENSIONS) / sizeof(const char*);

static const char* VK_REQUIRED_DEVICE_EXTENSIONS[] = { "VK_KHR_swapchain" };

static const uint32_t VK_NUM_REQUIRED_DEVICE_EXTENSIONS = sizeof(VK_REQUIRED_DEVICE_EXTENSIONS) / sizeof(const char*);

extern bool vkCreateSurfaceAPP(VkInstance inst, const VkAllocationCallbacks* alloc, VkSurfaceKHR* surface);

static bool vkCreateAndInitInstanceAPP(void* dll, const VkAllocationCallbacks* alloc, VkInstance* inst)
{
    vkGetInstanceProcAddr = appGetLibraryProc(dll, "vkGetInstanceProcAddr");
    TEST_RV(vkGetInstanceProcAddr, false, "ERROR: Failed to get pointer to vkGetInstanceProcAddr");
#define VULKAN_API_GOBAL(proc) \
    vk ## proc = ( PFN_vk ## proc )vkGetInstanceProcAddr( NULL, "vk" #proc ); \
    TEST_RV(vk ## proc, false, "ERROR: Failed to get pointer to vk" #proc );
#include "vk_api.inl"
    appPrintf(STR("Loaded global function pointers\n"));
    
	VkApplicationInfo appInfo;
	static char appName[APP_NAME_MAX];
	appTCharToUTF8(appName, gOptions->appName, APP_NAME_MAX);
	VK_INIT(appInfo, VK_STRUCTURE_TYPE_APPLICATION_INFO);
	appInfo.pApplicationName = appName;
	appInfo.apiVersion = VK_MAKE_VERSION(1, 1, 70);
	VkInstanceCreateInfo createInfo;
	VK_INIT(createInfo, VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO);
	createInfo.pApplicationInfo = &appInfo;
	
    createInfo.enabledLayerCount = VK_NUM_REQUIRED_LAYERS + gOptions->numLayers;
	if (createInfo.enabledLayerCount > VK_NUM_REQUIRED_LAYERS)
	{
		char** layers = (char**)malloc(createInfo.enabledLayerCount * sizeof(const char*)); // stack
		memcpy(layers, VK_REQUIRED_LAYERS, VK_NUM_REQUIRED_LAYERS * sizeof(const char*));
		memcpy(layers + VK_NUM_REQUIRED_LAYERS, gOptions->layers, gOptions->numLayers * sizeof(const char*));
		createInfo.ppEnabledLayerNames = (const char**)layers;
	}
	else
		createInfo.ppEnabledLayerNames = VK_REQUIRED_LAYERS;
#ifndef _MSC_VER
    appPrintf(STR("Instance debug layers:\n"));
    for (uint32_t i = 0; i < createInfo.enabledLayerCount; i++)
        appPrintf(STR("  %s\n"), createInfo.ppEnabledLayerNames[i]);
#endif
    
	createInfo.enabledExtensionCount = VK_NUM_REQUIRED_EXTENSIONS + gOptions->numExtensions;
	if (createInfo.enabledExtensionCount > VK_NUM_REQUIRED_EXTENSIONS)
	{
		char** ext = (char**)malloc(createInfo.enabledExtensionCount * sizeof(const char*)); // stack
		memcpy(ext, VK_REQUIRED_EXTENSIONS, VK_NUM_REQUIRED_EXTENSIONS * sizeof(const char*));
		memcpy(ext + VK_NUM_REQUIRED_EXTENSIONS, gOptions->extensions, gOptions->numExtensions * sizeof(const char*));
		createInfo.ppEnabledExtensionNames = (const char**)ext;
	}
	else
		createInfo.ppEnabledExtensionNames = VK_REQUIRED_EXTENSIONS;
#ifndef _MSC_VER
    appPrintf(STR("Instance extensions:\n"));
    for (uint32_t i = 0; i < createInfo.enabledExtensionCount; i++)
        appPrintf(STR("  %s\n"), createInfo.ppEnabledExtensionNames[i]);
#endif

    VkResult result = vkCreateInstance(&createInfo, alloc, inst);
	if (createInfo.enabledExtensionCount > VK_NUM_REQUIRED_EXTENSIONS)
		free((void*)createInfo.ppEnabledExtensionNames);
	if (createInfo.enabledLayerCount > VK_NUM_REQUIRED_LAYERS)
		free((void*)createInfo.ppEnabledLayerNames);
    
    if (result == VK_SUCCESS)
    {
#define VULKAN_API_INSTANCE(proc) \
        vk ## proc = ( PFN_vk ## proc )vkGetInstanceProcAddr( *inst, "vk" #proc ); \
        TEST_RV(vk ## proc, false, "ERROR: Failed to get pointer to vk" #proc );
#include "vk_api.inl"
        appPrintf(STR("Loaded instance-specific function pointers\n"));
    }
    else
        appPrintf(STR("ERROR: Failed to create Vulkan instance (%d)\n"), result);
	return true;
}

static bool vkGetAdapterAPP(VkInstance inst, VkSurfaceKHR surface, VkPhysicalDevice* adapter)
{
    *adapter = VK_NULL_HANDLE;
    uint32_t num = 0, idx = 0xff;
    if (vkEnumeratePhysicalDevices(inst, &num, NULL) == VK_SUCCESS && num)
    {
        VkPhysicalDevice* adapters = malloc(sizeof(VkPhysicalDevice) * num); // stack
        if (vkEnumeratePhysicalDevices(inst, &num, adapters) == VK_SUCCESS)
        {
            for (uint32_t i = 0; i < num; i++)
            {
                uint32_t numFamilies = 0;
                VkPhysicalDeviceProperties props;
                vkGetPhysicalDeviceProperties(adapters[i], &props);
                vkGetPhysicalDeviceQueueFamilyProperties(adapters[i], &numFamilies, 0);
#ifndef _MSC_VER
                appPrintf(STR("%u: %s (%u queue fam.)\n"), i, props.deviceName, numFamilies);
#endif
                if (*adapter == VK_NULL_HANDLE && props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
                {
                    VkBool32 canPresent = VK_FALSE;
                    for (uint32_t j = 0; j < numFamilies; j++)
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
        appPrintf(STR("Unsing adapter %u\n"), idx);
        return true;
    }
    return false;
}

static uint32_t vkGetSwapchainSizeAPP(VkEnvironment vkEnv)
{
    if (vkEnv->surfaceCaps.minImageCount > VK_SWAPCHAIN_SIZE)
        return vkEnv->surfaceCaps.minImageCount;
    if (vkEnv->surfaceCaps.maxImageCount < VK_SWAPCHAIN_SIZE)
        return vkEnv->surfaceCaps.maxImageCount;
    return VK_SWAPCHAIN_SIZE;
}

static VkPresentModeKHR vkGetSwapchainPresentMode(VkEnvironment vkEnv)
{
    for (uint32_t i = 0; i < vkEnv->numPresentModes; i++)
        if (vkEnv->presentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR)
            return VK_PRESENT_MODE_MAILBOX_KHR;
    return VK_PRESENT_MODE_FIFO_KHR;
}

bool vkRequestQueueAPP(VkEnvironment vkEnv, VkQueueFlags flags, bool present)
{
    uint32_t family = vkEnv->numQueueFamilies;
    if (vkEnv->numQueuesRequested < VK_MAX_QUEUES)
    {
        for (uint32_t i = 0; i < vkEnv->numQueueFamilies; i++)
        {
            if ((vkEnv->queueFamilies[i].queueFlags & flags) == flags
                && vkEnv->queueCount[i] < vkEnv->queueFamilies[i].queueCount)
            {
                if (present)
                {
                    VkBool32 canPresent = VK_FALSE;
                    vkGetPhysicalDeviceSurfaceSupportKHR(vkEnv->adapter, i, vkEnv->surface, &canPresent);
                    if (canPresent == VK_FALSE)
                        continue;
                }
                family = i;
                if (vkEnv->queueCount[family] == 0)
                    break;
            }
        }
    }
    else
    {
        appPrintf("ERROR: Too many device queue requests\n");
    }
    if (family < vkEnv->numQueueFamilies)
    {
        vkEnv->queueRequests[vkEnv->numQueuesRequested].family = family;
        vkEnv->queueRequests[vkEnv->numQueuesRequested].index = vkEnv->queueCount[family]++;
        ++vkEnv->numQueuesRequested;
        return true;
    }
    return false;
}

bool vkInitEnvironmentAPP(VkEnvironment* vkEnvPtr, const VkAllocationCallbacks* alloc)
{
    VkEnvironment vkEnv = calloc(1, sizeof(struct VkEnvironment));
	TEST_RV(vkEnv != NULL, false, "ERROR: Failed to allocate memory for global state");
    TEST_RV(appLoadLibrary(VK_LIBRARY, &vkEnv->library), false, "ERROR: Failed to load library");
	QTEST_RV(vkCreateAndInitInstanceAPP(vkEnv->library, alloc, &vkEnv->instance), false);
	QTEST_RV(vkCreateSurfaceAPP(vkEnv->instance, alloc, &vkEnv->surface), false);
	TEST_RV(vkGetAdapterAPP(vkEnv->instance, vkEnv->surface, &vkEnv->adapter), false, "ERROR: No compatible graphics adapter found");
	vkGetPhysicalDeviceQueueFamilyProperties(vkEnv->adapter, &vkEnv->numQueueFamilies, NULL);
	TEST_RV((vkEnv->numQueueFamilies>0)&&(vkEnv->numQueueFamilies<=VK_MAX_QUEUE_FAMILIES), false, "ERROR: Invalid numbed of queue families");
	vkGetPhysicalDeviceQueueFamilyProperties(vkEnv->adapter, &vkEnv->numQueueFamilies, vkEnv->queueFamilies);
	TEST_RV(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(vkEnv->adapter, vkEnv->surface, &vkEnv->surfaceCaps) == VK_SUCCESS, false, "ERROR: Failed to get surface capabilities");
    vkGetPhysicalDeviceSurfacePresentModesKHR(vkEnv->adapter, vkEnv->surface, &vkEnv->numPresentModes, NULL);
    vkGetPhysicalDeviceSurfacePresentModesKHR(vkEnv->adapter, vkEnv->surface, &vkEnv->numPresentModes, vkEnv->presentModes);
    *vkEnvPtr = vkEnv;
	return true;
}

bool vkCreateDeviceAndSwapchainAPP(VkEnvironment vkEnv, const VkAllocationCallbacks* alloc, VkDevice* device, VkSwapchainKHR* swapchain, VkQueue** queues)
{
    uint32_t familiesToCreate = 0;
    float priorities[] = {1.f, 1.f, 1.f, 1.f};
    VkDeviceQueueCreateInfo queueInfo[VK_MAX_QUEUE_FAMILIES];
    for (uint32_t i = 0; i < vkEnv->numQueueFamilies; i++)
    {
        if (vkEnv->queueCount[i] > 0)
        {
            VK_INIT(queueInfo[familiesToCreate], VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO);
            queueInfo[familiesToCreate].queueFamilyIndex = i;
            queueInfo[familiesToCreate].queueCount = vkEnv->queueCount[i];
            queueInfo[familiesToCreate].pQueuePriorities = priorities; // TODO: implement priorities
            ++familiesToCreate;
        }
    }
    VkDeviceCreateInfo createInfo;
    VK_INIT(createInfo, VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO);
    createInfo.pQueueCreateInfos = queueInfo;
    createInfo.queueCreateInfoCount = familiesToCreate;
    createInfo.enabledExtensionCount = VK_NUM_REQUIRED_DEVICE_EXTENSIONS;
    createInfo.ppEnabledExtensionNames = VK_REQUIRED_DEVICE_EXTENSIONS;
    TEST_RV(vkCreateDevice(vkEnv->adapter, &createInfo, alloc, device) == VK_SUCCESS, false, "ERROR: Failed to create device");
#define VULKAN_API_DEVICE(proc) \
    vk ## proc = ( PFN_vk ## proc )vkGetDeviceProcAddr( *device, "vk" #proc ); \
    TEST_RV(vk ## proc, false, "ERROR: Failed to get pointer to vk" #proc );
#include "vk_api.inl"
    appPrintf(STR("Loaded device-specific function pointers\n"));
    for (uint32_t i = 0; i < vkEnv->numQueuesRequested; i++)
    {
        uint32_t family = vkEnv->queueRequests[i].family;
        uint32_t index = vkEnv->queueRequests[i].index;
        VkQueue* queue = queues[i];
        vkGetDeviceQueue(*device, family, index, queue);
    }
    VkSwapchainCreateInfoKHR swapchainInfo;
    //VkPresentModeKHR;
    VK_INIT(swapchainInfo, VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR);
    swapchainInfo.minImageCount = vkGetSwapchainSizeAPP(vkEnv);
    swapchainInfo.presentMode = vkGetSwapchainPresentMode(vkEnv);
    return true;
}

void vkDestroyEnvironmentAPP(VkEnvironment vkEnv, const VkAllocationCallbacks* alloc)
{
    if (vkEnv->instance)
    {
        if (vkEnv->surface)
            vkDestroySurfaceKHR(vkEnv->instance, vkEnv->surface, alloc);
        vkDestroyInstance(vkEnv->instance, alloc);
    }
    appUnloadLibrary(vkEnv->library);
    free(vkEnv);
}
