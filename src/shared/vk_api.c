#include "vk_api.h"

#include "args.h"

#include <stdlib.h>
#include <string.h>

#define VK_MAX_QUEUES 8
#define VK_MAX_QUEUE_FAMILIES 8
#define VK_MAX_SURFACE_FORMATS 8

typedef struct
{
    uint32_t family;
    uint32_t index;
    VkQueue* outPtr;
} VkQueueReq;

//public
const VkAllocationCallbacks* gVkAlloc = NULL;
VkDevice gVkDev = VK_NULL_HANDLE;
VkSwapchainKHR gVkSwapchain = VK_NULL_HANDLE;
VkSurfaceFormatKHR gSurfaceFormat = {0, 0};

#define VK_MIN_BUFFER 65536u
static char* gVkMemBuffer = NULL;
static size_t gVkMemBufferSize = 0;
static size_t gVkMemBufferCurr = 0;
static void* gVkDllHandle = NULL;
static VkInstance gVkInst = VK_NULL_HANDLE;
static VkSurfaceKHR gVkSurf = VK_NULL_HANDLE;
static VkPhysicalDevice gVkPhDev = VK_NULL_HANDLE;
static uint32_t gNumQueueFamilies = 0;
static uint32_t gNumQueueRequests = 0;
static uint32_t gNumUsedFamilies = 0;
static uint32_t* gQueueCount = NULL;
static VkQueueFamilyProperties* gQueueFamProps = NULL;
static uint32_t gNumSurfFormats = 0;
static uint32_t gNumPresentModes = 0;
static VkSurfaceFormatKHR* gSurfFormats = NULL;
static VkPresentModeKHR* gPresentModes = NULL;
static VkSurfaceCapabilitiesKHR gSurfCaps;
static VkQueueReq* gOutQueues = NULL;
static uint32_t gNumBuffers = 0;

static PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr;
#define VULKAN_API_GOBAL(proc) PFN_vk ## proc vk ## proc = NULL;
#define VULKAN_API_INSTANCE(proc) PFN_vk ## proc vk ## proc = NULL;
#define VULKAN_API_DEVICE(proc) PFN_vk ## proc vk ## proc = NULL;
#include "vk_api.inl"

#define STACK_MARK(v) size_t _##v##_ = gVkMemBufferCurr
#define STACK_FREE(v) gVkMemBufferCurr = _##v##_
static void* stackAlloc(size_t size)
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
	appInfo.apiVersion = VK_MAKE_VERSION(1, 1, 0);
	VkInstanceCreateInfo createInfo;
	VK_INIT(createInfo, VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO);
	createInfo.pApplicationInfo = &appInfo;
    createInfo.enabledLayerCount = VK_NUM_REQUIRED_LAYERS + gOptions->numLayers;
	if (createInfo.enabledLayerCount > VK_NUM_REQUIRED_LAYERS)
	{
		char** layers = stackAlloc(createInfo.enabledLayerCount * sizeof(const char*));
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
		char** ext = stackAlloc(createInfo.enabledExtensionCount * sizeof(const char*));
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
        VkPhysicalDevice* adapters = stackAlloc(sizeof(VkPhysicalDevice) * num);
        if (vkEnumeratePhysicalDevices(gVkInst, &num, adapters) == VK_SUCCESS)
        {
            for (uint32_t i = 0; i < num; i++)
            {
                uint32_t numFamilies = 0;
                VkPhysicalDeviceProperties props;
                vkGetPhysicalDeviceProperties(adapters[i], &props);
                vkGetPhysicalDeviceQueueFamilyProperties(adapters[i], &numFamilies, NULL);
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
    vkGetPhysicalDeviceQueueFamilyProperties(gVkPhDev, &gNumQueueFamilies, NULL);
    gQueueCount = stackAlloc(gNumQueueFamilies * sizeof(uint32_t));
    memset(gQueueCount, 0, (gNumQueueFamilies * sizeof(uint32_t)));
    gQueueFamProps = stackAlloc(gNumQueueFamilies * sizeof(VkQueueFamilyProperties));
    vkGetPhysicalDeviceQueueFamilyProperties(gVkPhDev, &gNumQueueFamilies, gQueueFamProps);
    vkGetPhysicalDeviceSurfaceFormatsKHR(gVkPhDev, gVkSurf, &gNumSurfFormats, NULL);
    vkGetPhysicalDeviceSurfacePresentModesKHR(gVkPhDev, gVkSurf, &gNumPresentModes, NULL);
    gSurfFormats = stackAlloc(gNumSurfFormats * sizeof(VkSurfaceFormatKHR));
    gPresentModes = stackAlloc(gNumPresentModes * sizeof(VkPresentModeKHR));
    vkGetPhysicalDeviceSurfaceFormatsKHR(gVkPhDev, gVkSurf, &gNumSurfFormats, gSurfFormats);
    vkGetPhysicalDeviceSurfacePresentModesKHR(gVkPhDev, gVkSurf, &gNumPresentModes, gPresentModes);
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(gVkPhDev, gVkSurf, &gSurfCaps);
}

static uint32_t vkGetSwapchainSizeAPP()
{
    if (gSurfCaps.minImageCount > VK_SWAPCHAIN_SIZE)
        return gSurfCaps.minImageCount;
    if (gSurfCaps.maxImageCount < VK_SWAPCHAIN_SIZE)
        return gSurfCaps.maxImageCount;
    return VK_SWAPCHAIN_SIZE;
}

static VkPresentModeKHR vkGetSwapchainPresentMode()
{
    for (uint32_t i = 0; i < gNumPresentModes; i++)
        if (gPresentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR)
            return VK_PRESENT_MODE_MAILBOX_KHR;
    return VK_PRESENT_MODE_FIFO_KHR;
}

static VkSurfaceFormatKHR* vkGetSwapchainSurfaceFormat()
{
    static VkSurfaceFormatKHR defaultVal =
    {
        VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR
    };
    for (uint32_t i = 0; i < gNumSurfFormats; i++)
    {
        if (gSurfFormats[i].format == VK_FORMAT_B8G8R8A8_SRGB)
        {
            return &gSurfFormats[i];
        }
    }
    return &defaultVal;
}

static void vkRequestQueueAPP(VkQueueFlags flags, bool present, uint32_t* outFamily)
{
    uint32_t family = gNumQueueFamilies;
    for (uint32_t i = 0; i < gNumQueueFamilies; i++)
    {
        if ((gQueueFamProps[i].queueFlags & flags) == flags && gQueueCount[i] < gQueueFamProps[i].queueCount)
        {
            if (present)
            {
                VkBool32 canPresent = VK_FALSE;
                vkGetPhysicalDeviceSurfaceSupportKHR(gVkPhDev, i, gVkSurf, &canPresent);
                if (canPresent == VK_FALSE)
                    continue;
            }
            family = i;
            if (gQueueCount[i] == 0)
                break;
        }
    }
    ASSERT(family < gNumQueueFamilies, "ERROR: Failed to allocate device queue");   
    *outFamily = family;
    ++gQueueCount[family];
}

void vkRequestQueuesAPP(uint32_t count, VkQueueRequest* request)
{
    ASSERT(count, "ERROR: At least one GPU queue must be requested");
    gNumQueueRequests = count;
    gOutQueues = stackAlloc(count * sizeof(VkQueueReq));
    for (uint32_t i = 0; i < count; i++)
    {
        vkRequestQueueAPP(request[i].flags, request[i].present, request[i].outFamily);
        gOutQueues[i].family = *request[i].outFamily;
        gOutQueues[i].index = gQueueCount[*request[i].outFamily] - 1;
        gOutQueues[i].outPtr = request[i].outQueue;
        if (gOutQueues[i].index == 0)
            ++gNumUsedFamilies;
    }
}

void vkCreateDeviceAndSwapchainAPP()
{
    STACK_MARK(frame);
    ASSERT(gNumQueueRequests, "ERROR: At least one GPU queue must be requested");
    float* priorities = stackAlloc(gNumQueueRequests * sizeof(float));
    VkDeviceQueueCreateInfo* queueInfo = stackAlloc(gNumUsedFamilies * sizeof(VkDeviceQueueCreateInfo));
    for (uint32_t i = 0, j = 0; i < gNumQueueFamilies; i++)
    {
        if (gQueueCount[i] > 0)
        {
            VK_INIT(queueInfo[j], VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO);
            queueInfo[j].queueFamilyIndex = i;
            queueInfo[j].queueCount = gQueueCount[i];
            queueInfo[j].pQueuePriorities = priorities;
            for (uint32_t k = 0; k < gQueueCount[i]; k++)
            {
                *priorities = 1.f;
                priorities += 1;
            }
            ++j;
        }
    }
    VkDeviceCreateInfo createInfo;
    VK_INIT(createInfo, VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO);
    createInfo.pQueueCreateInfos = queueInfo;
    createInfo.queueCreateInfoCount = gNumUsedFamilies;
    createInfo.enabledExtensionCount = VK_NUM_REQUIRED_DEVICE_EXTENSIONS;
    createInfo.ppEnabledExtensionNames = VK_REQUIRED_DEVICE_EXTENSIONS;
    VK_ASSERT(vkCreateDevice(gVkPhDev, &createInfo, gVkAlloc, &gVkDev), "ERROR: Failed to create device");
#define VULKAN_API_DEVICE(proc) \
    vk ## proc = ( PFN_vk ## proc )vkGetDeviceProcAddr( gVkDev, "vk" #proc ); \
    ASSERT(vk ## proc, "ERROR: Failed to get pointer to vk" #proc );
#include "vk_api.inl"
    appPrintf(STR("Loaded device-specific function pointers\n"));
    for (uint32_t i = 0; i < gNumQueueRequests; i++)
    {
        uint32_t family = gOutQueues[i].family;
        uint32_t index = gOutQueues[i].index;
        VkQueue* queue = gOutQueues[i].outPtr;
        vkGetDeviceQueue(gVkDev, family, index, queue);
    }
    VkSwapchainCreateInfoKHR swapchainInfo;
    gSurfaceFormat = *vkGetSwapchainSurfaceFormat();
    VK_INIT(swapchainInfo, VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR);
    swapchainInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    swapchainInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapchainInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapchainInfo.imageArrayLayers = 1;
    swapchainInfo.clipped = VK_TRUE;
    swapchainInfo.preTransform = gSurfCaps.currentTransform;
    swapchainInfo.minImageCount = vkGetSwapchainSizeAPP();
    swapchainInfo.presentMode = vkGetSwapchainPresentMode();
    swapchainInfo.imageFormat = gSurfaceFormat.format;
    swapchainInfo.imageColorSpace = gSurfaceFormat.colorSpace;
    swapchainInfo.surface = gVkSurf;
    swapchainInfo.imageExtent = gSurfCaps.currentExtent;
    VK_ASSERT(vkCreateSwapchainKHR(gVkDev, &swapchainInfo, gVkAlloc, &gVkSwapchain), "ERROR: Failed to create swapchain");
    gNumBuffers = swapchainInfo.minImageCount;
    STACK_FREE(frame);
}

void vkCreateCommandBufferAPP(VkCommandBufferAllocateInfo* info, VkCommandBuffer** out)
{
    if (info->commandBufferCount == 0)
        info->commandBufferCount = gNumBuffers;
    VkCommandBuffer* res = stackAlloc(info->commandBufferCount * sizeof(VkCommandBuffer));
    VK_ASSERT(vkAllocateCommandBuffers(gVkDev, info, res), "ERROR: Failed to allocate command buffers");
    *out = res;
}

void vkDestroyCommandBufferAPP(VkCommandPool pool, uint32_t count, VkCommandBuffer* ptr)
{
    count = (count) ? count : gNumBuffers;
    vkFreeCommandBuffers(gVkDev, pool, count, ptr);
}

void vkCreateSemaphoreAPP(VkSemaphore** out, uint32_t count)
{
    count = (count) ? count : gNumBuffers;
    VkSemaphoreCreateInfo createInfo =
    {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO, .pNext = NULL, .flags = 0
    };
    VkSemaphore* semaphores = stackAlloc(count);
    for (uint32_t i = 0; i < count; i++)
    {
        VK_ASSERT(vkCreateSemaphore(gVkDev, &createInfo, gVkAlloc, &semaphores[i]), "ERROR: Failed to create semaphore");
    }
    *out = semaphores;
}

void vkDestroySemaphoreAPP(VkSemaphore* in, uint32_t count)
{
    count = (count) ? count : gNumBuffers;
    for (uint32_t i = 0; i < count; i++)
    {
        vkDestroySemaphore(gVkDev, in[i], gVkAlloc);
    }
}

void vkAcquireNextImageAPP(VkSemaphore sem, uint32_t* image)
{
    VkAcquireNextImageInfoKHR info =
    {
        .sType = VK_STRUCTURE_TYPE_ACQUIRE_NEXT_IMAGE_INFO_KHR,
        .pNext = NULL, .swapchain = gVkSwapchain, .timeout = UINT64_MAX,
        .semaphore = sem, .fence = VK_NULL_HANDLE, .deviceMask = 0
    };
    VK_ASSERT(vkAcquireNextImage2KHR(gVkDev, &info, image), "ERROR: Failed to acquire image");
}

uint32_t vkNextFrameAPP(uint32_t current)
{
    return ((current + 1) % gNumBuffers);
}

void vkInitializeAPP(size_t maxMem, const VkAllocationCallbacks* alloc)
{
    gVkAlloc = alloc;
    maxMem = (maxMem) ? ALIGN16(maxMem) : VK_MIN_BUFFER;
    gVkMemBufferSize = (maxMem < VK_MIN_BUFFER) ? VK_MIN_BUFFER : maxMem;
    gVkMemBuffer = (char*)malloc(gVkMemBufferSize);
    ASSERT(gVkMemBuffer, "ERROR: Failed to allocate internal memory");
    ASSERT(appLoadLibrary(VK_LIBRARY, &gVkDllHandle), "ERROR: Failed to load library");
    vkCreateAndInitInstanceAPP();
    ASSERT_Q(vkCreateSurfaceAPP(gVkInst, gVkAlloc, &gVkSurf));
    vkGetGraphicsAdapterAPP();
}

void vkFinalizeAPP(void)
{
    vkDeviceWaitIdle(gVkDev);
    vkDestroySwapchainKHR(gVkDev, gVkSwapchain, gVkAlloc);
    vkDestroySurfaceKHR(gVkInst, gVkSurf, gVkAlloc);
    vkDestroyInstance(gVkInst, gVkAlloc);
    appUnloadLibrary(gVkDllHandle);
    free(gVkMemBuffer);
}
