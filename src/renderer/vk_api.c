#include "../global.h"
#include "../platform/args.h"

#include "vk_api.h"

#include <string.h>

#ifdef _MSC_VER
#define VK_LIBRARY "vulkan-1.dll"
#else
#define VK_LIBRARY "@rpath/libvulkan.1.dylib"
#endif

#define VKFN(c) TEST_Q( (c) == VK_SUCCESS )

static const char* VK_REQUIRED_LAYERS[] = { "VK_LAYER_LUNARG_standard_validation" };
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

#define VK_SWAPCHAIN_MIN_SIZE 3
#define VK_CPU_MEM_TOTAL 16384u // 3/4 forward, 1/4 stack
#define VK_CPU_MEM_FORWD ((VK_CPU_MEM_TOTAL >> 1) + (VK_CPU_MEM_TOTAL >> 2))
#define VK_CPU_MEM_STACK (VK_CPU_MEM_TOTAL - VK_CPU_MEM_FORWD)

typedef struct VkQueueInfo
{
    uint32_t family;
    uint32_t index;
    VkQueue queue;
} VkQueueInfo;

struct VkContextImpl
{
    PFN_vkGetInstanceProcAddr GetInstanceProcAddrImpl;
#define VULKAN_API_GOBAL(proc) PFN_vk ## proc proc ## Impl;
#define VULKAN_API_INSTANCE(proc) PFN_vk ## proc proc ## Impl;
#define VULKAN_API_DEVICE(proc) PFN_vk ## proc proc ## Impl;
#include "vk_api.inl"
    MemAlloc mem;
    void* dll;
    VkAllocationCallbacks* alloc;
    VkInstance inst;
#ifdef _DEBUG
    VkDebugReportCallbackEXT debug;
#endif
    VkSurfaceKHR surf;
    VkSurfaceFormatKHR surfFmt;
    VkPhysicalDevice phdev;
    VkPhysicalDeviceMemoryProperties memProps;

    uint32_t phdMask; // physical device mask
    uint32_t scSize;  // number of buffers in the swapchain

    VkExtent2D fbSize;
    VkDevice dev;
    VkPipelineCache plCache;
    VkQueueInfo* queues;

    VkSwapchainKHR swapchain;
    VkImage* fbImage; // array holding swapchain images
    VkImageView* fbView;
    VkSemaphore* frmFbOk; // semaphores to signify image acquisition
    VkSemaphore* frmFinished; // semaphores to signal to present
    VkFence* frmFence;
    uint32_t frameIdx; // current frame index

    struct
    {
        uint32_t image;
        VkSemaphore fbOk, finished;
        VkFence fence;
    } frame;

    struct
    {
        VkCommandPool pool;
        VkCommandBuffer* buffer;
    } cmd;
};

extern const void* sysGetVkSurfaceInfo(void);
static void vk_RequestQueue(VkContext vk, VkQueueFamilyProperties* qfp, uint32_t* cnt, uint32_t num, VkQueueRequest* req, uint32_t* fam);
static void vk_CreateAndInitInstance(VkContext vk, Options opts);
static VkBool32 vk_DebugFn(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objectType, uint64_t object, size_t location, int32_t messageCode, const char* pLayerPrefix, const char* pMessage, void* pUserData);
static void vk_GetGraphicsAdapter(VkContext vk);
static void vk_CreateDeviceAndSwapchain(VkContext vk, const uint32_t* queueCount, uint32_t numFamilies);
static VkSurfaceFormatKHR vk_GetSwapchainSurfaceFormat(VkContext vk);
static VkPresentModeKHR vk_GetSwapchainPresentMode(VkContext vk);
static uint32_t vk_GetSwapchainSize(VkSurfaceCapabilitiesKHR* caps);

void vk_CreateContext(MemAlloc mem, const VkContextInfo* info, VkContext* vkPtr)
{
    ASSERT_Q(info->parent == NULL);
    size_t memBytes = mem_SubAllocSize(VK_CPU_MEM_TOTAL);
    void* parentMem = mem_ForwdAlloc(mem, memBytes);
    MemAlloc local = MemAllocCreate(VK_CPU_MEM_FORWD, VK_CPU_MEM_STACK, parentMem, memBytes);
    VkContext vk = mem_ForwdAlloc(local, sizeof(struct VkContextImpl));
    vk->mem = local;
    vk->alloc = NULL;
    vk->frameIdx = 0;
    mem_StackFramePush(vk->mem);
    vk_CreateAndInitInstance(vk, info->options);
#if defined(VK_USE_PLATFORM_MACOS_MVK)
    VKFN(vk->CreateMacOSSurfaceMVKImpl(vk->inst, sysGetVkSurfaceInfo(), vk->alloc, &vk->surf));
#elif defined(VK_USE_PLATFORM_WIN32_KHR)
    VKFN(vk->CreateWin32SurfaceKHRImpl(vk->inst, sysGetVkSurfaceInfo(), vk->alloc, &vk->surf));
#endif
    vk_GetGraphicsAdapter(vk);
    uint32_t numQueueFamilies = 0;
    vk->GetPhysicalDeviceQueueFamilyPropertiesImpl(vk->phdev, &numQueueFamilies, NULL);
    VkQueueFamilyProperties* qProps = mem_StackAlloc(vk->mem, numQueueFamilies * sizeof(VkQueueFamilyProperties));
    uint32_t* qCount = mem_StackAlloc(vk->mem, numQueueFamilies * sizeof(uint32_t));
    vk->queues = mem_ForwdAlloc(vk->mem, info->numQueueReq * sizeof(VkQueueInfo));
    vk->GetPhysicalDeviceQueueFamilyPropertiesImpl(vk->phdev, &numQueueFamilies, qProps);
    memset(qCount, 0, (numQueueFamilies * sizeof(uint32_t)));
    for (uint32_t i = 0; i < info->numQueueReq; i++)
    {
        uint32_t familyIdx = INV_IDX;
        vk_RequestQueue(vk, qProps, qCount, numQueueFamilies, &info->queueReq[i], &familyIdx);
        vk->queues[i].family = familyIdx;
        vk->queues[i].index = qCount[familyIdx] - 1;
    }
    vk_CreateDeviceAndSwapchain(vk, qCount, numQueueFamilies);
    VkCommandPoolCreateInfo poolInfo = { 0 };
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    VKFN(vk->CreateCommandPoolImpl(vk->dev, &poolInfo, vk->alloc, &vk->cmd.pool));
    vk->cmd.buffer = mem_ForwdAlloc(vk->mem, vk->scSize * sizeof(VkCommandBuffer));
    VkCommandBufferAllocateInfo cbInfo = { 0 };
    cbInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    cbInfo.commandPool = vk->cmd.pool;
    cbInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    cbInfo.commandBufferCount = vk->scSize;
    VKFN(vk->AllocateCommandBuffersImpl(vk->dev, &cbInfo, vk->cmd.buffer));
    mem_StackFramePop(vk->mem);
    *vkPtr = vk;
}

void vk_DeviceWaitIdle(VkContext vk)
{
    vk->DeviceWaitIdleImpl(vk->dev);
}

void vk_DestroyContext(VkContext vk)
{
    vk->DeviceWaitIdleImpl(vk->dev);
    for (uint32_t i = 0; i < vk->scSize; i++)
    {
        vk->DestroySemaphoreImpl(vk->dev, vk->frmFbOk[i], vk->alloc);
        vk->DestroySemaphoreImpl(vk->dev, vk->frmFinished[i], vk->alloc);
        vk->DestroyFenceImpl(vk->dev, vk->frmFence[i], vk->alloc);
        vk->DestroyImageViewImpl(vk->dev, vk->fbView[i], vk->alloc);
    }
    vk->FreeCommandBuffersImpl(vk->dev, vk->cmd.pool, vk->scSize, vk->cmd.buffer);
    vk->DestroyCommandPoolImpl(vk->dev, vk->cmd.pool, vk->alloc);
    vk->DestroySwapchainKHRImpl(vk->dev, vk->swapchain, vk->alloc);
    vk->DestroyDeviceImpl(vk->dev, vk->alloc);
    vk->DestroySurfaceKHRImpl(vk->inst, vk->surf, vk->alloc);
#ifdef _DEBUG
    vk->DestroyDebugReportCallbackEXTImpl(vk->inst, vk->debug, vk->alloc);
#endif
    vk->DestroyInstanceImpl(vk->inst, vk->alloc);
    sysUnloadLibrary(vk->dll);
}

VkFormat vk_GetSwapchainImageFormat(VkContext vk)
{
    return vk->surfFmt.format;
}

void vk_BeginFrame(VkContext vk)
{
    vk->frame.image = INV_IDX;
    vk->frame.fbOk = vk->frmFbOk[vk->frameIdx];
    vk->frame.finished = vk->frmFinished[vk->frameIdx];
    vk->frame.fence = vk->frmFence[vk->frameIdx];
    VKFN(vk->WaitForFencesImpl(vk->dev, 1, &vk->frame.fence, VK_TRUE, UINT64_MAX));
    VKFN(vk->ResetFencesImpl(vk->dev, 1, &vk->frame.fence));
    VKFN(vk->AcquireNextImageKHRImpl(vk->dev, vk->swapchain, UINT64_MAX, vk->frame.fbOk, VK_NULL_HANDLE, &vk->frame.image));
    VkCommandBufferBeginInfo info = { 0 };
    info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    VKFN(vk->BeginCommandBufferImpl(vk->cmd.buffer[vk->frameIdx], &info));
}

VkCommandBuffer vk_GetPrimaryCommandBuffer(VkContext vk)
{
    return vk->cmd.buffer[vk->frameIdx];
}

void vk_BeginCommandBuffer(VkContext vk, VkCommandBuffer cb, const VkCommandBufferBeginInfo* info)
{
    VKFN(vk->BeginCommandBufferImpl(cb, info));
}

void vk_EndCommandBuffer(VkContext vk, VkCommandBuffer cb)
{
    VKFN(vk->EndCommandBufferImpl(cb));
}

void vk_SubmitFrame(VkContext vk, uint32_t queue)
{
    VKFN(vk->EndCommandBufferImpl(vk->cmd.buffer[vk->frameIdx]));
    VkSubmitInfo sInfo = { 0 };
    sInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    sInfo.waitSemaphoreCount = 1;
    sInfo.pWaitSemaphores = &vk->frame.fbOk;
    sInfo.pWaitDstStageMask = (VkPipelineStageFlags[]) { VK_PIPELINE_STAGE_ALL_COMMANDS_BIT };
    sInfo.commandBufferCount = 1;
    sInfo.pCommandBuffers = &vk->cmd.buffer[vk->frameIdx];
    sInfo.signalSemaphoreCount = 1;
    sInfo.pSignalSemaphores = &vk->frame.finished;
    VKFN(vk->QueueSubmitImpl(vk->queues[queue].queue, 1, &sInfo, vk->frame.fence));
    VkPresentInfoKHR pInfo = { 0 };
    pInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    pInfo.waitSemaphoreCount = 1;
    pInfo.pWaitSemaphores = &vk->frame.finished;
    pInfo.swapchainCount = 1;
    pInfo.pSwapchains = &vk->swapchain;
    pInfo.pImageIndices = &vk->frame.image;
    VKFN(vk->QueuePresentKHRImpl(vk->queues[queue].queue, &pInfo));
    uint32_t nextIndex = vk->frameIdx + 1;
    vk->frameIdx = (nextIndex == vk->scSize) ? 0 : nextIndex;
}

void vk_CreateDescriptorPool(VkContext vk, const VkDescriptorPoolCreateInfo* info, VkDescriptorPool* pool)
{
    VKFN(vk->CreateDescriptorPoolImpl(vk->dev, info, vk->alloc, pool));
}

void vk_DestroyDescriptorPool(VkContext vk, VkDescriptorPool pool)
{
    vk->DestroyDescriptorPoolImpl(vk->dev, pool, vk->alloc);
}

void vk_RequestQueue(VkContext vk, VkQueueFamilyProperties* qfp, uint32_t* cnt, uint32_t num, VkQueueRequest* req, uint32_t* fam)
{
    uint32_t family = num;
    for (uint32_t i = 0; i < num; i++)
    {
        if ((qfp[i].queueFlags & req->flags) == req->flags && cnt[i] < qfp[i].queueCount)
        {
            if (req->present)
            {
                VkBool32 canPresent = VK_FALSE;
                vk->GetPhysicalDeviceSurfaceSupportKHRImpl(vk->phdev, i, vk->surf, &canPresent);
                if (canPresent == VK_FALSE)
                    continue;
            }
            family = i;
            if (cnt[i] == 0)
                break;
        }
    }
    ASSERT(family < num, "ERROR: %s", "Failed to allocate device queue");
    *fam = family;
    ++cnt[family];
}

void vk_CreateAndInitInstance(VkContext vk, Options opts)
{
    TEST(sysLoadLibrary(VK_LIBRARY, &vk->dll), "ERROR: %s", "Failed to load library");
    mem_StackFramePush(vk->mem);
    vk->GetInstanceProcAddrImpl = sysGetLibraryProc(vk->dll, "vkGetInstanceProcAddr");
    ASSERT(vk->GetInstanceProcAddrImpl, "ERROR: Failed to get pointer to %s", "vkGetInstanceProcAddr");
#define VULKAN_API_GOBAL(proc) \
    vk->proc ## Impl = ( PFN_vk ## proc )vk->GetInstanceProcAddrImpl( NULL, "vk" #proc ); \
    ASSERT(vk->proc ## Impl, "ERROR: Failed to get pointer to %s", "vk" #proc );
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
        char** layers = mem_StackAlloc(vk->mem, createInfo.enabledLayerCount * sizeof(const char*));
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
        char** ext = mem_StackAlloc(vk->mem, createInfo.enabledExtensionCount * sizeof(const char*));
        memcpy(ext, VK_REQUIRED_EXTENSIONS, VK_NUM_REQUIRED_EXTENSIONS * sizeof(const char*));
        memcpy(ext + VK_NUM_REQUIRED_EXTENSIONS, opts->extensions, opts->numExtensions * sizeof(const char*));
        createInfo.ppEnabledExtensionNames = (const char**)ext;
    }
    else
        createInfo.ppEnabledExtensionNames = VK_REQUIRED_EXTENSIONS;
    sysPrintf("Instance extensions:\n");
    for (uint32_t i = 0; i < createInfo.enabledExtensionCount; i++)
        sysPrintf("  %s\n", createInfo.ppEnabledExtensionNames[i]);
    VkResult result = vk->CreateInstanceImpl(&createInfo, vk->alloc, &vk->inst);
    if (result == VK_SUCCESS)
    {
#define VULKAN_API_INSTANCE(proc) \
        vk->proc ## Impl = ( PFN_vk ## proc )vk->GetInstanceProcAddrImpl( vk->inst, "vk" #proc ); \
        ASSERT(vk->proc ## Impl, "ERROR: Failed to get pointer to %s", "vk" #proc );
#include "vk_api.inl"
        sysPrintf("Loaded instance-specific function pointers\n");
#ifdef _DEBUG
        VkDebugReportCallbackCreateInfoEXT debugInfo = { 0 };
        debugInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
        debugInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
        debugInfo.pfnCallback = &vk_DebugFn;
        debugInfo.pUserData = vk;
        VKFN(vk->CreateDebugReportCallbackEXTImpl(vk->inst, &debugInfo, vk->alloc, &vk->debug));
#endif
    }
    else
        ASSERT(false, "ERROR: %s", "Failed to create Vulkan instance");
    mem_StackFramePop(vk->mem);
}

VkBool32 vk_DebugFn(VkDebugReportFlagsEXT flags,
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

void vk_GetGraphicsAdapter(VkContext vk)
{
    mem_StackFramePush(vk->mem);
    vk->phdev = VK_NULL_HANDLE;
    uint32_t num = 0, idx = INV_IDX, fallback = INV_IDX;
    if (vk->EnumeratePhysicalDevicesImpl(vk->inst, &num, NULL) == VK_SUCCESS && num)
    {
        VkPhysicalDevice* adapters = mem_StackAlloc(vk->mem, sizeof(VkPhysicalDevice) * num);
        if (vk->EnumeratePhysicalDevicesImpl(vk->inst, &num, adapters) == VK_SUCCESS)
        {
            for (uint32_t i = 0; i < num; i++)
            {
                uint32_t numFamilies = 0;
                VkPhysicalDeviceProperties props;
                vk->GetPhysicalDevicePropertiesImpl(adapters[i], &props);
                vk->GetPhysicalDeviceQueueFamilyPropertiesImpl(adapters[i], &numFamilies, NULL);
                sysPrintf("%u: %s (%u queue families)\n", i, props.deviceName, numFamilies);
                VkBool32 canPresent = VK_FALSE;
                for (uint32_t j = 0; j < numFamilies; j++)
                {
                    vk->GetPhysicalDeviceSurfaceSupportKHRImpl(adapters[i], j, vk->surf, &canPresent);
                    if (canPresent == VK_TRUE)
                    {
                        if (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
                        {
                            idx = i;
                            break;
                        }
                        else if (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU && fallback == INV_IDX)
                            fallback = i;
                    }
                }
            }
            if (idx == INV_IDX)
                if (fallback != INV_IDX)
                    idx = fallback;
            if (idx != INV_IDX)
                vk->phdev = adapters[idx];
        }
    }
    ASSERT(vk->phdev, "ERROR: %s", "Failed to choose graphics adapter");
    vk->GetPhysicalDeviceMemoryPropertiesImpl(vk->phdev, &vk->memProps);
    sysPrintf("Using adapter %u\n", idx);
    mem_StackFramePop(vk->mem);
    vk->phdMask = 1 << idx;
}

void vk_CreateDeviceAndSwapchain(VkContext vk, const uint32_t* queueCount, uint32_t numFamilies)
{
    uint32_t numQueues = 0;
    mem_StackFramePush(vk->mem);
    VkDeviceCreateInfo info = { 0 };
    VkDeviceQueueCreateInfo* qInfo = mem_StackAlloc(vk->mem, numFamilies * sizeof(VkDeviceQueueCreateInfo));
    for (uint32_t i = 0; i < numFamilies; i++)
    {
        if (queueCount[i] > 0)
        {
            qInfo[i].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            qInfo[i].pNext = NULL;
            qInfo[i].flags = 0;
            qInfo[i].queueFamilyIndex = i;
            qInfo[i].queueCount = queueCount[i];
            float* prios = mem_StackAlloc(vk->mem, queueCount[i] * sizeof(float));
            for (uint32_t j = 0; j < queueCount[i]; j++)
                prios[j] = 1.f;
            qInfo[i].pQueuePriorities = prios;
            info.queueCreateInfoCount += 1;
            numQueues += queueCount[i];
        }
    }
    info.pQueueCreateInfos = qInfo;
    info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    info.enabledExtensionCount = VK_NUM_REQUIRED_DEVICE_EXTENSIONS;
    info.ppEnabledExtensionNames = VK_REQUIRED_DEVICE_EXTENSIONS;
    VKFN(vk->CreateDeviceImpl(vk->phdev, &info, vk->alloc, &vk->dev));
#define VULKAN_API_DEVICE(proc) \
    vk->proc ## Impl = ( PFN_vk ## proc )vk->GetDeviceProcAddrImpl( vk->dev, "vk" #proc ); \
    ASSERT(vk->proc ## Impl, "ERROR: Failed to get pointer to %s", "vk" #proc );
#include "vk_api.inl"
    sysPrintf("Loaded device-specific function pointers\n");
    for (uint32_t i = 0; i < numQueues; i++)
    {
        uint32_t family = vk->queues[i].family;
        uint32_t index = vk->queues[i].index;
        VkQueue* queue = &vk->queues[i].queue;
        vk->GetDeviceQueueImpl(vk->dev, family, index, queue);
    }
    VkSwapchainCreateInfoKHR swapchainInfo = { 0 };
    VkSurfaceCapabilitiesKHR surfCaps;
    vk->GetPhysicalDeviceSurfaceCapabilitiesKHRImpl(vk->phdev, vk->surf, &surfCaps);
    vk->surfFmt = vk_GetSwapchainSurfaceFormat(vk);
    swapchainInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchainInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    swapchainInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapchainInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapchainInfo.imageArrayLayers = 1;
    swapchainInfo.clipped = VK_TRUE;
    swapchainInfo.preTransform = surfCaps.currentTransform;
    swapchainInfo.minImageCount = vk_GetSwapchainSize(&surfCaps);
    swapchainInfo.presentMode = vk_GetSwapchainPresentMode(vk);
    swapchainInfo.imageFormat = vk->surfFmt.format;
    swapchainInfo.imageColorSpace = vk->surfFmt.colorSpace;
    swapchainInfo.surface = vk->surf;
    swapchainInfo.imageExtent = surfCaps.currentExtent;
    VKFN(vk->CreateSwapchainKHRImpl(vk->dev, &swapchainInfo, vk->alloc, &vk->swapchain));
    VKFN(vk->GetSwapchainImagesKHRImpl(vk->dev, vk->swapchain, &vk->scSize, NULL));
    vk->fbSize = surfCaps.currentExtent;
    vk->fbImage = mem_ForwdAlloc(vk->mem, vk->scSize * sizeof(VkImage));
    vk->fbView = mem_ForwdAlloc(vk->mem, vk->scSize * sizeof(VkImageView));
    vk->frmFbOk = mem_ForwdAlloc(vk->mem, vk->scSize * sizeof(VkSemaphore));
    vk->frmFinished = mem_ForwdAlloc(vk->mem, vk->scSize * sizeof(VkSemaphore));
    vk->frmFence = mem_ForwdAlloc(vk->mem, vk->scSize * sizeof(VkFence));
    vk->GetSwapchainImagesKHRImpl(vk->dev, vk->swapchain, &vk->scSize, vk->fbImage);
    for (uint32_t i = 0; i < vk->scSize; i++)
    {
        VkSemaphoreCreateInfo sInfo = { 0 };
        sInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        VkFenceCreateInfo fInfo = { 0 };
        fInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
        VKFN(vk->CreateFenceImpl(vk->dev, &fInfo, vk->alloc, &vk->frmFence[i]));
        VKFN(vk->CreateSemaphoreImpl(vk->dev, &sInfo, vk->alloc, &vk->frmFbOk[i]));
        VKFN(vk->CreateSemaphoreImpl(vk->dev, &sInfo, vk->alloc, &vk->frmFinished[i]));
        VkImageViewCreateInfo ivInfo = { 0 };
        ivInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        ivInfo.image = vk->fbImage[i];
        ivInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        ivInfo.format = vk->surfFmt.format;
        ivInfo.components.r = ivInfo.components.g = ivInfo.components.b = ivInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        ivInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        ivInfo.subresourceRange.baseMipLevel = 0;
        ivInfo.subresourceRange.levelCount = 1;
        ivInfo.subresourceRange.baseArrayLayer = 0;
        ivInfo.subresourceRange.layerCount = 1;
        VKFN(vk->CreateImageViewImpl(vk->dev, &ivInfo, vk->alloc, &vk->fbView[i]));
    }
    mem_StackFramePop(vk->mem);
}

VkSurfaceFormatKHR vk_GetSwapchainSurfaceFormat(VkContext vk)
{
    mem_StackFramePush(vk->mem);
    VkSurfaceFormatKHR retVal =
    {
        VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR
    };
    uint32_t numFormats = 0;
    VKFN(vk->GetPhysicalDeviceSurfaceFormatsKHRImpl(vk->phdev, vk->surf, &numFormats, NULL));
    VkSurfaceFormatKHR* formats = mem_StackAlloc(vk->mem, numFormats * sizeof(VkSurfaceFormatKHR));
    vk->GetPhysicalDeviceSurfaceFormatsKHRImpl(vk->phdev, vk->surf, &numFormats, formats);
    for (uint32_t i = 0; i < numFormats; i++)
    {
        if (formats[i].format == VK_FORMAT_B8G8R8A8_SRGB)
        {
            retVal = formats[i];
            break;
        }
    }
    mem_StackFramePop(vk->mem);
    return retVal;
}

VkPresentModeKHR vk_GetSwapchainPresentMode(VkContext vk)
{
    mem_StackFramePush(vk->mem);
    uint32_t numModes = 0;
    VkPresentModeKHR retVal = VK_PRESENT_MODE_FIFO_KHR;
    VKFN(vk->GetPhysicalDeviceSurfacePresentModesKHRImpl(vk->phdev, vk->surf, &numModes, NULL));
    VkPresentModeKHR* modes = mem_StackAlloc(vk->mem, numModes * sizeof(VkPresentModeKHR));
    vk->GetPhysicalDeviceSurfacePresentModesKHRImpl(vk->phdev, vk->surf, &numModes, modes);
    for (uint32_t i = 0; i < numModes; i++)
        if (modes[i] == VK_PRESENT_MODE_MAILBOX_KHR)
        {
            retVal = modes[i];
            break;
        }
    mem_StackFramePop(vk->mem);
    return retVal;
}

uint32_t vk_GetSwapchainSize(VkSurfaceCapabilitiesKHR* caps)
{
    if (caps->minImageCount > VK_SWAPCHAIN_MIN_SIZE)
        return caps->minImageCount;
    if (caps->maxImageCount < VK_SWAPCHAIN_MIN_SIZE)
        return caps->maxImageCount;
    return VK_SWAPCHAIN_MIN_SIZE;
}

struct VkTexture2DImpl
{
    uint32_t width, height;
    VkImageView id;
};

struct VkRenderPassDataImpl
{
    uint32_t numAttachments;
    struct
    {
        uint32_t num;
        uint32_t width;
        uint32_t height;
        VkFramebuffer* id;
    } fb;
    VkClearValue* clearVal;
    VkRenderPass id;
};

void vk_CreateRenderPass(VkContext vk, const VkRenderPassCreateInfo* info, VkRenderPassData* pass)
{
    VkRenderPassData tmp = mem_ForwdAlloc(vk->mem, sizeof(struct VkRenderPassDataImpl));
    VKFN(vk->CreateRenderPassImpl(vk->dev, info, vk->alloc, &tmp->id));
    tmp->numAttachments = info->attachmentCount;
    tmp->clearVal = mem_ForwdAlloc(vk->mem, sizeof(VkClearValue) * tmp->numAttachments);
    memset(tmp->clearVal, 0, sizeof(VkClearValue) * tmp->numAttachments);
    *pass = tmp;
}

void vk_SetClearColorValue(VkRenderPassData pass, uint32_t att, float value[4])
{
    pass->clearVal[att].color.float32[0] = value[0];
    pass->clearVal[att].color.float32[1] = value[1];
    pass->clearVal[att].color.float32[2] = value[2];
    pass->clearVal[att].color.float32[3] = value[3];
}

void vk_DestroyRenderPass(VkContext vk, VkRenderPassData pass)
{
    vk->DestroyRenderPassImpl(vk->dev, pass->id, vk->alloc);
    for (uint32_t i = 0; i < pass->fb.num; i++)
        vk->DestroyFramebufferImpl(vk->dev, pass->fb.id[i], vk->alloc);
}

void vk_CmdBeginRenderPass(VkContext vk, VkCommandBuffer cb, VkRenderPassData pass)
{
    VkRenderPassBeginInfo info = { 0 };
    info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    info.renderPass = pass->id;
    info.framebuffer = (pass->fb.num != 1) ? pass->fb.id[vk->frame.image] : *pass->fb.id;
    info.renderArea.extent.width = pass->fb.width;
    info.renderArea.extent.height = pass->fb.height;
    info.clearValueCount = 1;
    info.pClearValues = pass->clearVal;
    vk->CmdBeginRenderPassImpl(cb, &info, VK_SUBPASS_CONTENTS_INLINE);
}

void vk_CmdEndRenderPass(VkContext vk, VkCommandBuffer cb)
{
    vk->CmdEndRenderPassImpl(cb);
}

void vk_InitPassFramebuffer(VkContext vk, VkRenderPassData pass, const VkTexture2D* views)
{
    pass->fb.num = 1;
    mem_StackFramePush(vk->mem);
    ASSERT_Q((views) || (pass->numAttachments == 1));
    uint32_t scImageIndex = INV_IDX, fbWidth = UINT32_MAX, fbHeight = UINT32_MAX;
    VkImageView* copy = mem_StackAlloc(vk->mem, pass->numAttachments * sizeof(VkImageView));
    for (uint32_t i = 0; i < pass->numAttachments; i++)
    {
        copy[i] = (views && views[i]) ? views[i]->id : VK_NULL_HANDLE;
        uint32_t w = (views && views[i]) ? views[i]->width : vk->fbSize.width;
        uint32_t h = (views && views[i]) ? views[i]->height : vk->fbSize.height;
        if (copy[i] == VK_NULL_HANDLE)
        {
            scImageIndex = i;
            pass->fb.num = vk->scSize;
        }
        fbHeight = (fbHeight > h) ? h : fbHeight;
        fbWidth = (fbWidth > w) ? w : fbWidth;
    }
    pass->fb.width = fbWidth;
    pass->fb.height = fbHeight;
    pass->fb.id = mem_ForwdAlloc(vk->mem, sizeof(VkFramebuffer) * pass->fb.num);
    VkFramebufferCreateInfo info = { 0 };
    info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    info.renderPass = pass->id;
    info.attachmentCount = pass->numAttachments;
    info.pAttachments = copy;
    info.width = fbWidth;
    info.height = fbHeight;
    info.layers = 1;
    for (uint32_t i = 0; i < pass->fb.num; i++)
    {
        if (scImageIndex != INV_IDX)
            copy[scImageIndex] = vk->fbView[i];
        VKFN(vk->CreateFramebufferImpl(vk->dev, &info, vk->alloc, &pass->fb.id[i]));
    }
    mem_StackFramePop(vk->mem);
}

void vk_MallocBuffer(VkContext vk, VkBuffer buff, VkMemoryPropertyFlags flags)
{
    VkMemoryRequirements reqs;
    uint32_t type = VK_MAX_MEMORY_TYPES;
    VkDeviceMemory result = VK_NULL_HANDLE;
    vk->GetBufferMemoryRequirementsImpl(vk->dev, buff, &reqs);
    for (uint32_t i = 0; i < vk->memProps.memoryTypeCount; i++)
    {
        if ((reqs.memoryTypeBits & (1 << i))
            && ((vk->memProps.memoryTypes[i].propertyFlags & flags) == flags))
        {
            type = i;
            break;
        }
    }
    ASSERT_Q(type < vk->memProps.memoryTypeCount);
    VkMemoryAllocateInfo allocInfo;
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.pNext = NULL;
    allocInfo.allocationSize = reqs.size;
    allocInfo.memoryTypeIndex = type;
    VKFN(vk->AllocateMemoryImpl(vk->dev, &allocInfo, vk->alloc, &result));
    VKFN(vk->BindBufferMemoryImpl(vk->dev, buff, result, 0));
}
