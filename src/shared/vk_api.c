#include "vk_api.h"

#include "args.h"

#include <stdlib.h>
#include <string.h>

#define VK_MAX_QUEUES 8
#define VK_MAX_QUEUE_FAMILIES 8
#define VK_MAX_SURFACE_FORMATS 8

//public
VkAllocationCallbacks* gVkAlloc = NULL;
VkDevice gVkDev = VK_NULL_HANDLE;
VkSwapchainKHR gVkSwapchain = VK_NULL_HANDLE;

//private: instance, library handle
#define VK_MIN_BUFFER 65536u
static char* gVkMemBuffer = NULL;
static size_t gVkMemBufferSize = 0;
static size_t gVkMemBufferCurr = 0;
static void* gVkDllHandle = NULL;
static VkInstance gVkInst = VK_NULL_HANDLE;
static VkSurfaceKHR gVkSurf = VK_NULL_HANDLE;
static VkPhysicalDevice gVkPhDev = VK_NULL_HANDLE;

static PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr;
#define VULKAN_API_GOBAL(proc) PFN_vk ## proc vk ## proc = NULL;
#define VULKAN_API_INSTANCE(proc) PFN_vk ## proc vk ## proc = NULL;
#define VULKAN_API_DEVICE(proc) PFN_vk ## proc vk ## proc = NULL;
#include "vk_api.inl"

#define STACK_MARK(v) size_t _##v##_ = gVkMemBufferCurr
#define STACK_FREE(v) gVkMemBufferCurr = _##v##_
static void* tmpBuffAlloc(size_t size)
{
    size = ALIGN16(size);
    ASSERT(gVkMemBufferSize - gVkMemBufferCurr >= size, "ERROR: Internal stack out of memory");
    void* retVal = gVkMemBuffer + gVkMemBufferCurr;
    gVkMemBufferCurr += size;
    return retVal;
}

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
    uint32_t numSurfaceFormats;
    VkSurfaceFormatKHR* surfaceFormat;
    VkSurfaceFormatKHR surfaceFormats[VK_MAX_SURFACE_FORMATS];
    VkDebugReportCallbackEXT debugCallback;
};

static const uint32_t VK_NUM_REQUIRED_EXTENSIONS = sizeof(VK_REQUIRED_EXTENSIONS) / sizeof(const char*);

static const char* VK_REQUIRED_DEVICE_EXTENSIONS[] = { "VK_KHR_swapchain" };

static const uint32_t VK_NUM_REQUIRED_DEVICE_EXTENSIONS = sizeof(VK_REQUIRED_DEVICE_EXTENSIONS) / sizeof(const char*);

extern bool vkCreateSurfaceAPP(VkInstance inst, const VkAllocationCallbacks* alloc, VkSurfaceKHR* surface);

static void vkCreateAndInitInstanceAPP()
{
    STACK_MARK(frame);
    vkGetInstanceProcAddr = appGetLibraryProc(gVkDllHandle, "vkGetInstanceProcAddr");
    ASSERT(vkGetInstanceProcAddr, "ERROR: Failed to get pointer to vkGetInstanceProcAddr");
#define VULKAN_API_GOBAL(proc) \
    vk ## proc = ( PFN_vk ## proc )vkGetInstanceProcAddr( NULL, "vk" #proc ); \
    ASSERT(vk ## proc, "ERROR: Failed to get pointer to vk" #proc );
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
		char** layers = tmpBuffAlloc(createInfo.enabledLayerCount * sizeof(const char*));
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
		char** ext = tmpBuffAlloc(createInfo.enabledExtensionCount * sizeof(const char*));
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
    VkResult result = vkCreateInstance(&createInfo, gVkAlloc, &gVkInst);
    if (result == VK_SUCCESS)
    {
#define VULKAN_API_INSTANCE(proc) \
        vk ## proc = ( PFN_vk ## proc )vkGetInstanceProcAddr( gVkInst, "vk" #proc ); \
        ASSERT(vk ## proc, "ERROR: Failed to get pointer to vk" #proc );
#include "vk_api.inl"
        appPrintf(STR("Loaded instance-specific function pointers\n"));
    }
    else
        ASSERT(false, "ERROR: Failed to create Vulkan instance");
    STACK_FREE(frame);
}

static void vkGetGraphicsAdapterAPP()
{
    STACK_MARK(frame);
    uint32_t num = 0, idx = 0xff, fallback = 0xff;
    if (vkEnumeratePhysicalDevices(gVkInst, &num, NULL) == VK_SUCCESS && num)
    {
        VkPhysicalDevice* adapters = tmpBuffAlloc(sizeof(VkPhysicalDevice) * num);
        if (vkEnumeratePhysicalDevices(gVkInst, &num, adapters) == VK_SUCCESS)
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
                VkBool32 canPresent = VK_FALSE;
                for (uint32_t j = 0; j < numFamilies; j++)
                {
                    vkGetPhysicalDeviceSurfaceSupportKHR(adapters[i], j, gVkSurf, &canPresent);
                    if (canPresent == VK_TRUE)
                    {
                        if (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
                        {
                            idx = i;
                            break;
                        }
                        else if (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU && fallback == 0xFF)
                            fallback = i;
                    }
                }
            }
            if (idx == 0xff)
                if (fallback != 0xff)
                    idx = fallback;
            gVkPhDev = (idx == 0xff) ? VK_NULL_HANDLE : adapters[idx];
        }
    }
    ASSERT(gVkPhDev, "ERROR: Failed to choose graphics adapter");
    appPrintf(STR("Unsing adapter %u\n"), idx);
    STACK_FREE(frame);
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

static VkSurfaceFormatKHR* vkGetSwapchainSurfaceFormat(VkEnvironment vkEnv)
{
    static VkSurfaceFormatKHR defaultVal =
    {
        VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR
    };
    for (uint32_t i = 0; i < vkEnv->numSurfaceFormats; i++)
    {
        if (vkEnv->surfaceFormats[i].format == VK_FORMAT_B8G8R8A8_SRGB)
        {
            vkEnv->surfaceFormat = &vkEnv->surfaceFormats[i];
            return &vkEnv->surfaceFormats[i];
        }
    }
    vkEnv->surfaceFormat = &defaultVal;
    return &defaultVal;
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
    /*VkEnvironment vkEnv = calloc(1, sizeof(struct VkEnvironment));
	TEST_RV(vkEnv != NULL, false, "ERROR: Failed to allocate memory for global state");
    TEST_RV(appLoadLibrary(VK_LIBRARY, &vkEnv->library), false, "ERROR: Failed to load library");
	QTEST_RV(vkCreateAndInitInstanceAPP(vkEnv->library, alloc, &vkEnv->instance, &vkEnv->debugCallback), false);
	QTEST_RV(vkCreateSurfaceAPP(vkEnv->instance, alloc, &vkEnv->surface), false);
	TEST_RV(vkGetAdapterAPP(vkEnv->instance, vkEnv->surface, &vkEnv->adapter), false, "ERROR: No compatible graphics adapter found");
	vkGetPhysicalDeviceQueueFamilyProperties(vkEnv->adapter, &vkEnv->numQueueFamilies, NULL);
	TEST_RV((vkEnv->numQueueFamilies)&&(vkEnv->numQueueFamilies<=VK_MAX_QUEUE_FAMILIES), false, "ERROR: Invalid number of queue families");
	vkGetPhysicalDeviceQueueFamilyProperties(vkEnv->adapter, &vkEnv->numQueueFamilies, vkEnv->queueFamilies);
	TEST_RV(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(vkEnv->adapter, vkEnv->surface, &vkEnv->surfaceCaps) == VK_SUCCESS, false, "ERROR: Failed to get surface capabilities");
    vkGetPhysicalDeviceSurfacePresentModesKHR(vkEnv->adapter, vkEnv->surface, &vkEnv->numPresentModes, NULL);
    TEST_RV(vkEnv->numPresentModes, false, "ERROR: invalid number of supported present modes");
    vkGetPhysicalDeviceSurfacePresentModesKHR(vkEnv->adapter, vkEnv->surface, &vkEnv->numPresentModes, vkEnv->presentModes);
    vkGetPhysicalDeviceSurfaceFormatsKHR(vkEnv->adapter, vkEnv->surface, &vkEnv->numSurfaceFormats, NULL);
    TEST_RV((vkEnv->numSurfaceFormats)&&(vkEnv->numSurfaceFormats < VK_MAX_SURFACE_FORMATS), false, "ERROR: invalid number of surface formats");
    vkGetPhysicalDeviceSurfaceFormatsKHR(vkEnv->adapter, vkEnv->surface, &vkEnv->numSurfaceFormats, vkEnv->surfaceFormats);
    *vkEnvPtr = vkEnv;*/
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
    VkSurfaceFormatKHR* surfaceFmt = vkGetSwapchainSurfaceFormat(vkEnv);
    VK_INIT(swapchainInfo, VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR);
    swapchainInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    swapchainInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapchainInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapchainInfo.imageArrayLayers = 1;
    swapchainInfo.clipped = VK_TRUE;
    swapchainInfo.preTransform = vkEnv->surfaceCaps.currentTransform;
    swapchainInfo.minImageCount = vkGetSwapchainSizeAPP(vkEnv);
    swapchainInfo.presentMode = vkGetSwapchainPresentMode(vkEnv);
    swapchainInfo.imageFormat = surfaceFmt->format;
    swapchainInfo.imageColorSpace = surfaceFmt->colorSpace;
    swapchainInfo.surface = vkEnv->surface;
    swapchainInfo.imageExtent = vkEnv->surfaceCaps.currentExtent;
    TEST_RV(vkCreateSwapchainKHR(*device, &swapchainInfo, alloc, swapchain) == VK_SUCCESS, false, "ERROR: Failed to create swapchain");
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

void vkInitialize(size_t maxMem)
{
    maxMem = (maxMem) ? ALIGN16(maxMem) : VK_MIN_BUFFER;
    gVkMemBufferSize = (maxMem < VK_MIN_BUFFER) ? VK_MIN_BUFFER : maxMem;
    gVkMemBuffer = (char*)malloc(gVkMemBufferSize);
    ASSERT(gVkMemBuffer, "ERROR: Failed to allocate internal memory");
    ASSERT(appLoadLibrary(VK_LIBRARY, &gVkDllHandle), "ERROR: Failed to load library");
    vkCreateAndInitInstanceAPP();
    ASSERT_Q(vkCreateSurfaceAPP(gVkInst, gVkAlloc, &gVkSurf));
    vkGetGraphicsAdapterAPP();
}

void vkFinalize(void)
{
    vkDestroySurfaceKHR(gVkInst, gVkSurf, gVkAlloc);
    vkDestroyInstance(gVkInst, gVkAlloc);
    appUnloadLibrary(gVkDllHandle);
    free(gVkMemBuffer);
}
