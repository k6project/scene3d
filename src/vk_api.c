#include "global.h"

#include "vk_api.h"

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
} VkxQueueReqImpl;

const VkAllocationCallbacks* gVkAlloc = NULL;
VkDevice gVkDev = VK_NULL_HANDLE;
VkSwapchainKHR gVkSwapchain = VK_NULL_HANDLE;
VkSurfaceFormatKHR gSurfaceFormat = {0, 0};
VkImage* gDisplayImage = NULL;

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
static VkxQueueReqImpl* gOutQueues = NULL;
static uint32_t gNumBuffers = 0;
static uint32_t gPhDevMask = 0;
static VkDebugReportCallbackEXT gDebug = VK_NULL_HANDLE;

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
    ASSERT(gVkMemBufferSize - gVkMemBufferCurr >= size, "ERROR: %s", "Internal stack out of memory");
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

extern bool sysCreateVkSurface(VkInstance inst, const VkAllocationCallbacks* alloc, VkSurfaceKHR* surface);

#ifdef _DEBUG

static VkBool32 vkDebugAPP(VkDebugReportFlagsEXT flags,
						   VkDebugReportObjectTypeEXT objectType,
						   uint64_t object,
	                       size_t location,
	                       int32_t messageCode,
	                       const char* pLayerPrefix,
	                       const char* pMessage,
	                       void* pUserData)
{
	sysPrintf("%s\n", pMessage);
	return VK_FALSE;
}

#endif

static void vkCreateAndInitInstanceAPP(const Options* opts)
{
    STACK_MARK(frame);
    vkGetInstanceProcAddr = sysGetLibraryProc(gVkDllHandle, "vkGetInstanceProcAddr");
    ASSERT(vkGetInstanceProcAddr, "ERROR: Failed to get pointer to %s", "vkGetInstanceProcAddr");
#define VULKAN_API_GOBAL(proc) \
    vk ## proc = ( PFN_vk ## proc )vkGetInstanceProcAddr( NULL, "vk" #proc ); \
    ASSERT(vk ## proc, "ERROR: Failed to get pointer to %s", "vk" #proc );
#include "vk_api.inl"
    sysPrintf("Loaded global function pointers\n");
	VkApplicationInfo appInfo;
    memset(&appInfo, 0, sizeof(appInfo));
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = opts->appName;
	appInfo.apiVersion = VK_MAKE_VERSION(1, 1, 0);
	VkInstanceCreateInfo createInfo;
	memset(&createInfo, 0, sizeof(createInfo));
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;
    createInfo.enabledLayerCount = VK_NUM_REQUIRED_LAYERS + opts->numLayers;
	if (createInfo.enabledLayerCount > VK_NUM_REQUIRED_LAYERS)
	{
		char** layers = stackAlloc(createInfo.enabledLayerCount * sizeof(const char*));
		memcpy(layers, VK_REQUIRED_LAYERS, VK_NUM_REQUIRED_LAYERS * sizeof(const char*));
		memcpy(layers + VK_NUM_REQUIRED_LAYERS, opts->layers, opts->numLayers * sizeof(const char*));
		createInfo.ppEnabledLayerNames = (const char**)layers;
	}
	else
		createInfo.ppEnabledLayerNames = VK_REQUIRED_LAYERS;
    sysPrintf("Instance debug layers:\n");
    for (uint32_t i = 0; i < createInfo.enabledLayerCount; i++)
        sysPrintf("  %s\n", createInfo.ppEnabledLayerNames[i]);
	createInfo.enabledExtensionCount = VK_NUM_REQUIRED_EXTENSIONS + opts->numExtensions;
	if (createInfo.enabledExtensionCount > VK_NUM_REQUIRED_EXTENSIONS)
	{
		char** ext = stackAlloc(createInfo.enabledExtensionCount * sizeof(const char*));
		memcpy(ext, VK_REQUIRED_EXTENSIONS, VK_NUM_REQUIRED_EXTENSIONS * sizeof(const char*));
		memcpy(ext + VK_NUM_REQUIRED_EXTENSIONS, opts->extensions, opts->numExtensions * sizeof(const char*));
		createInfo.ppEnabledExtensionNames = (const char**)ext;
	}
	else
		createInfo.ppEnabledExtensionNames = VK_REQUIRED_EXTENSIONS;
    sysPrintf("Instance extensions:\n");
    for (uint32_t i = 0; i < createInfo.enabledExtensionCount; i++)
        sysPrintf("  %s\n", createInfo.ppEnabledExtensionNames[i]);
    VkResult result = vkCreateInstance(&createInfo, gVkAlloc, &gVkInst);
    if (result == VK_SUCCESS)
    {
#define VULKAN_API_INSTANCE(proc) \
        vk ## proc = ( PFN_vk ## proc )vkGetInstanceProcAddr( gVkInst, "vk" #proc ); \
        ASSERT(vk ## proc, "ERROR: Failed to get pointer to %s", "vk" #proc );
#include "vk_api.inl"
        sysPrintf("Loaded instance-specific function pointers\n");
#ifdef _DEBUG
		VkDebugReportCallbackCreateInfoEXT debugInfo =
		{
			.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT,
			.pNext = NULL, .flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT,
            .pfnCallback = &vkDebugAPP, .pUserData = NULL
		};
		VK_ASSERT_Q(vkCreateDebugReportCallbackEXT(gVkInst, &debugInfo, gVkAlloc, &gDebug));
#endif
    }
    else
        ASSERT(false, "ERROR: %s", "Failed to create Vulkan instance");
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
                sysPrintf("%u: %s (%u queue fam.)\n", i, props.deviceName, numFamilies);
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
    ASSERT(gVkPhDev, "ERROR: %s", "Failed to choose graphics adapter");
    sysPrintf("Unsing adapter %u\n", idx);
    STACK_FREE(frame);
	gPhDevMask = 1 << idx;
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
    ASSERT(family < gNumQueueFamilies, "ERROR: %s", "Failed to allocate device queue");
    *outFamily = family;
    ++gQueueCount[family];
}

void vkxRequestQueues(uint32_t count, VkxQueueReq* request)
{
    ASSERT(count, "ERROR: %s", "At least one GPU queue must be requested");
    gNumQueueRequests = count;
    gOutQueues = stackAlloc(count * sizeof(VkxQueueReqImpl));
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

void vkxCreateDeviceAndSwapchain()
{
    STACK_MARK(frame);
    ASSERT(gNumQueueRequests, "ERROR: %s", "At least one GPU queue must be requested");
    float* priorities = stackAlloc(gNumQueueRequests * sizeof(float));
    VkDeviceQueueCreateInfo* queueInfo = stackAlloc(gNumUsedFamilies * sizeof(VkDeviceQueueCreateInfo));
    for (uint32_t i = 0, j = 0; i < gNumQueueFamilies; i++)
    {
        if (gQueueCount[i] > 0)
        {
            memset(&queueInfo[j], 0, sizeof(queueInfo[j]));
            queueInfo[j].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
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
    memset(&createInfo, 0, sizeof(createInfo));
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.pQueueCreateInfos = queueInfo;
    createInfo.queueCreateInfoCount = gNumUsedFamilies;
    createInfo.enabledExtensionCount = VK_NUM_REQUIRED_DEVICE_EXTENSIONS;
    createInfo.ppEnabledExtensionNames = VK_REQUIRED_DEVICE_EXTENSIONS;
    VK_ASSERT(vkCreateDevice(gVkPhDev, &createInfo, gVkAlloc, &gVkDev), "ERROR: %s", "Failed to create device");
#define VULKAN_API_DEVICE(proc) \
    vk ## proc = ( PFN_vk ## proc )vkGetDeviceProcAddr( gVkDev, "vk" #proc ); \
    ASSERT(vk ## proc, "ERROR: Failed to get pointer to %s", "vk" #proc );
#include "vk_api.inl"
    sysPrintf("Loaded device-specific function pointers\n");
    for (uint32_t i = 0; i < gNumQueueRequests; i++)
    {
        uint32_t family = gOutQueues[i].family;
        uint32_t index = gOutQueues[i].index;
        VkQueue* queue = gOutQueues[i].outPtr;
        vkGetDeviceQueue(gVkDev, family, index, queue);
    }
    VkSwapchainCreateInfoKHR swapchainInfo;
    gSurfaceFormat = *vkGetSwapchainSurfaceFormat();
    memset(&swapchainInfo, 0, sizeof(swapchainInfo));
    swapchainInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
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
    VK_ASSERT(vkCreateSwapchainKHR(gVkDev, &swapchainInfo, gVkAlloc, &gVkSwapchain), "ERROR: %s", "Failed to create swapchain");
	VK_ASSERT(vkGetSwapchainImagesKHR(gVkDev, gVkSwapchain, &gNumBuffers, NULL), "ERROR: %s", "Failed to get swapchain images");
	STACK_FREE(frame);
	gDisplayImage = stackAlloc(gNumBuffers * sizeof(VkImage));
	vkGetSwapchainImagesKHR(gVkDev, gVkSwapchain, &gNumBuffers, gDisplayImage);
}

void vkxCreateCommandPool(uint32_t queueFamily, VkCommandPool* pool)
{
    VkCommandPoolCreateInfo createInfo =
    {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO, .pNext = NULL ,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT, .queueFamilyIndex = queueFamily
    };
    VK_ASSERT(vkCreateCommandPool(gVkDev, &createInfo, gVkAlloc, pool), "ERROR: %s", "Failed to create command pool");
}

void vkxCreateCommandBuffer(VkCommandPool pool, VkCommandBufferLevel level, uint32_t count, VkCommandBuffer** cbuff)
{
    VkCommandBufferAllocateInfo info =
    {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO, .pNext = NULL,
        .commandPool = pool ,.level = level , .commandBufferCount = count
    };
    if (info.commandBufferCount == 0)
        info.commandBufferCount = gNumBuffers;
    VkCommandBuffer* res = stackAlloc(info.commandBufferCount * sizeof(VkCommandBuffer));
    VK_ASSERT(vkAllocateCommandBuffers(gVkDev, &info, res), "ERROR: %s", "Failed to allocate command buffers");
    *cbuff = res;
}

void vkxDestroyCommandBuffer(VkCommandPool pool, uint32_t count, VkCommandBuffer* ptr)
{
    count = (count) ? count : gNumBuffers;
    vkFreeCommandBuffers(gVkDev, pool, count, ptr);
}

void vkxCreateSemaphore(VkSemaphore** out, uint32_t count)
{
    count = (count) ? count : gNumBuffers;
    VkSemaphoreCreateInfo createInfo =
    {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO, .pNext = NULL, .flags = 0
    };
    VkSemaphore* semaphores = stackAlloc(count * sizeof(VkSemaphore));
    for (uint32_t i = 0; i < count; i++)
    {
        VK_ASSERT(vkCreateSemaphore(gVkDev, &createInfo, gVkAlloc, &semaphores[i]), "ERROR: %s", "Failed to create semaphore");
    }
    *out = semaphores;
}

void vkxDestroySemaphore(VkSemaphore* sem, uint32_t count)
{
    count = (count) ? count : gNumBuffers;
    for (uint32_t i = 0; i < count; i++)
    {
        vkDestroySemaphore(gVkDev, sem[i], gVkAlloc);
    }
}

void vkxCreateFence(VkFence** out, uint32_t count)
{
	count = (count) ? count : gNumBuffers;
	VkFenceCreateInfo createInfo =
	{
		.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO, .pNext = NULL, .flags = VK_FENCE_CREATE_SIGNALED_BIT
	};
	VkFence* fences = stackAlloc(count * sizeof(VkFence));
	for (uint32_t i = 0; i < count; i++)
	{
		VK_ASSERT(vkCreateFence(gVkDev, &createInfo, gVkAlloc, &fences[i]), "ERROR: %s", "Failed to create fence");
	}
	*out = fences;
}

void vkxDestroyFence(VkFence* fen, uint32_t count)
{
	count = (count) ? count : gNumBuffers;
	for (uint32_t i = 0; i < count; i++)
	{
		vkDestroyFence(gVkDev, fen[i], gVkAlloc);
	}
}

void vkxAcquireNextImage(VkSemaphore sem, uint32_t* image)
{
#if 0
    VkAcquireNextImageInfoKHR info =
    {
        .sType = VK_STRUCTURE_TYPE_ACQUIRE_NEXT_IMAGE_INFO_KHR,
        .pNext = NULL, .swapchain = gVkSwapchain, .timeout = UINT64_MAX,
        .semaphore = sem, .fence = VK_NULL_HANDLE, .deviceMask = gPhDevMask
    };
	VkResult result = vkAcquireNextImage2KHR(gVkDev, &info, image);
#else
	VkResult result = vkAcquireNextImageKHR(gVkDev, gVkSwapchain, UINT64_MAX, sem, VK_NULL_HANDLE, image);
#endif
    VK_ASSERT(result, "ERROR: Failed to acquire image (%d)", result);
}

uint32_t vkxNextFrame(uint32_t current)
{
    return ((current + 1) % gNumBuffers);
}

void vkxInitialize(size_t maxMem, const Options* opts, const VkAllocationCallbacks* alloc)
{
    gVkAlloc = alloc;
    maxMem = (maxMem) ? ALIGN16(maxMem) : VK_MIN_BUFFER;
    gVkMemBufferSize = (maxMem < VK_MIN_BUFFER) ? VK_MIN_BUFFER : maxMem;
    gVkMemBuffer = malloc(gVkMemBufferSize);
    ASSERT(gVkMemBuffer, "ERROR: %s", "Failed to allocate internal memory");
    ASSERT(sysLoadLibrary(VK_LIBRARY, &gVkDllHandle), "ERROR: %s", "Failed to load library");
    vkCreateAndInitInstanceAPP(opts);
    ASSERT_Q(sysCreateVkSurface(gVkInst, gVkAlloc, &gVkSurf));
    vkGetGraphicsAdapterAPP();
}

void vkxFinalize(void)
{
    vkDeviceWaitIdle(gVkDev);
    vkDestroySwapchainKHR(gVkDev, gVkSwapchain, gVkAlloc);
    vkDestroyDevice(gVkDev, gVkAlloc);
    vkDestroySurfaceKHR(gVkInst, gVkSurf, gVkAlloc);
#ifdef _DEBUG
	vkDestroyDebugReportCallbackEXT(gVkInst, gDebug, gVkAlloc);
#endif
    vkDestroyInstance(gVkInst, gVkAlloc);
    sysUnloadLibrary(gVkDllHandle);
    free(gVkMemBuffer);
}

void vkxCmdClearColorImage(VkCmdBufferInfo info, VkImage img, VkClearColorValue* color)
{
    VkCommandBuffer cmdBuff = info.commandBuffer;
    uint32_t queueFamily = info.queueFamily;
    VkImageMemoryBarrier clearBarrier =
    {
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        .pNext = NULL,
        .srcAccessMask = 0,
        .dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
        .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        .srcQueueFamilyIndex = queueFamily,
        .dstQueueFamilyIndex = queueFamily,
        .image = img,
        .subresourceRange =
        {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1
        }
    };
    vkCmdPipelineBarrier(cmdBuff, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, NULL, 0, NULL, 1, &clearBarrier);
    vkCmdClearColorImage(cmdBuff, img, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, color, 1, &clearBarrier.subresourceRange);
}

void vkxCmdPreparePresent(VkCmdBufferInfo info, VkImage img)
{
    VkCommandBuffer cmdBuff = info.commandBuffer;
    uint32_t queueFamily = info.queueFamily;
    VkImageMemoryBarrier presentBarrier =
    {
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        .pNext = NULL,
        .srcAccessMask = 0,
        .dstAccessMask = 0,
        .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
        .srcQueueFamilyIndex = queueFamily,
        .dstQueueFamilyIndex = queueFamily,
        .image = img,
        .subresourceRange =
        {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1
        }
    };
    vkCmdPipelineBarrier(cmdBuff, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0, 0, NULL, 0, NULL, 1, &presentBarrier);
}
