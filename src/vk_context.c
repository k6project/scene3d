#include "global.h"

#include "vk_context.h"
#include "args.h"

#include <string.h>

#ifdef _MSC_VER
#define VK_LIBRARY "vulkan-1.dll"
#else
#define VK_LIBRARY "@rpath/libvulkan.1.dylib"
#endif

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
#define VK_CPU_MEM_TOTAL 65536u // 3/4 forward, 1/4 stack
#define VK_CPU_MEM_FORWD ((VK_CPU_MEM_TOTAL >> 1) + (VK_CPU_MEM_TOTAL >> 2))
#define VK_CPU_MEM_STACK (VK_CPU_MEM_TOTAL - VK_CPU_MEM_FORWD)

extern bool sysCreateVkSurface(VkContext* vk);
static void vk_RequestQueue(VkContext* vk, VkQueueFamilyProperties* qfp, uint32_t* cnt, uint32_t num, VkQueueRequest* req, uint32_t* fam);
static void vk_CreateAndInitInstance(VkContext* vk, const Options* opts);
static VkBool32 vk_DebugFn(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objectType, uint64_t object, size_t location, int32_t messageCode, const char* pLayerPrefix, const char* pMessage, void* pUserData);
static void vk_GetGraphicsAdapter(VkContext* vk);
static void vk_CreateDeviceAndSwapchain(VkContext* vk, const uint32_t* queueCount, uint32_t numFamilies);
static VkSurfaceFormatKHR vk_GetSwapchainSurfaceFormat(VkContext* vk);
static VkPresentModeKHR vk_GetSwapchainPresentMode(VkContext* vk);
static uint32_t vk_GetSwapchainSize(VkSurfaceCapabilitiesKHR* caps);

void vk_CreateContextImpl(VkContext** ctx, const VkContextInfo* info, HMemAlloc mem)
{
	ASSERT_Q(info->parent == NULL);
	size_t memBytes = memSubAllocSize(VK_CPU_MEM_TOTAL);
	void* parentMem = memForwdAlloc(mem, memBytes);
	HMemAlloc local = memAllocCreate(VK_CPU_MEM_FORWD, VK_CPU_MEM_STACK, parentMem, memBytes);
	VkContext* vk = memForwdAlloc(local, sizeof(VkContext));
	vk->mem = local;
	vk->alloc = NULL;
	memStackFramePush(vk->mem);
	vk_CreateAndInitInstance(vk, info->options);
	TEST_Q(sysCreateVkSurface(vk));
	vk_GetGraphicsAdapter(vk);
	uint32_t numQueueFamilies = 0;
	vk->GetPhysicalDeviceQueueFamilyPropertiesImpl(vk->phdev, &numQueueFamilies, NULL);
	VkQueueFamilyProperties* qProps = memStackAlloc(vk->mem, numQueueFamilies * sizeof(VkQueueFamilyProperties));
	uint32_t* qCount = memStackAlloc(vk->mem, numQueueFamilies * sizeof(uint32_t));
	vk->queues = memForwdAlloc(vk->mem, info->numQueueReq * sizeof(VkQueueInfo));
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
	memStackFramePop(vk->mem);
	*ctx = vk;
}

void vk_DestroyContextImpl(VkContext** ctx)
{
	VkContext* vk = *ctx;
	vk->DeviceWaitIdleImpl(vk->dev);
    for (uint32_t i = 0; i < vk->scSize; i++)
    {
        vk->DestroySemaphoreImpl(vk->dev, vk->frmFbOk[i], vk->alloc);
        vk->DestroySemaphoreImpl(vk->dev, vk->frmFinished[i], vk->alloc);
        vk->DestroyFenceImpl(vk->dev, vk->frmFence[i], vk->alloc);
		vk->DestroyImageViewImpl(vk->dev, vk->fbView[i], vk->alloc);
    }
	vk->DestroySwapchainKHRImpl(vk->dev, vk->swapchain, vk->alloc);
	vk->DestroyDeviceImpl(vk->dev, vk->alloc);
	vk->DestroySurfaceKHRImpl(vk->inst, vk->surf, vk->alloc);
#ifdef _DEBUG
	vk->DestroyDebugReportCallbackEXTImpl(vk->inst, vk->debug, vk->alloc);
#endif
	vk->DestroyInstanceImpl(vk->inst, vk->alloc);
	sysUnloadLibrary(vk->dll);
	*ctx = NULL;
}

void vk_BeginFrame(VkContext* vk, VkFrame* frm)
{
    frm->index = vk->frameIdx;
	frm->imgIdx = INV_IDX;
	frm->fbImage = VK_NULL_HANDLE;
	frm->fbOk = vk->frmFbOk[vk->frameIdx];
	frm->finished = vk->frmFinished[vk->frameIdx];
	frm->fence = vk->frmFence[vk->frameIdx];
	vk->WaitForFencesImpl(vk->dev, 1, &frm->fence, VK_TRUE, UINT64_MAX);
	vk->ResetFencesImpl(vk->dev, 1, &frm->fence);
}

void vk_EndFrame(VkContext* vk, VkFrame* frm)
{
	uint32_t nextIndex = vk->frameIdx + 1;
	vk->frameIdx = (nextIndex == vk->scSize) ? 0 : nextIndex;
}

void vk_InitCommandRecorder(VkContext* vk, VkCommandRecorder* cr, uint32_t queueIdx)
{
    VkCommandPoolCreateInfo info = {0};
    info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    VKFN(vk->CreateCommandPoolImpl(vk->dev, &info, vk->alloc, &cr->pool));
    cr->buffers = memForwdAlloc(vk->mem, vk->scSize * sizeof(VkCommandBuffer));
    VkCommandBufferAllocateInfo cbInfo = {0};
    cbInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    cbInfo.commandPool = cr->pool;
    cbInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    cbInfo.commandBufferCount = vk->scSize;
    VKFN(vk->AllocateCommandBuffersImpl(vk->dev, &cbInfo, cr->buffers));
}

void vk_DestroyCommandRecorder(VkContext* vk, VkCommandRecorder* cr)
{
    vk->FreeCommandBuffersImpl(vk->dev, cr->pool, vk->scSize, cr->buffers);
    vk->DestroyCommandPoolImpl(vk->dev, cr->pool, vk->alloc);
}

void vk_RequestQueue(VkContext* vk, VkQueueFamilyProperties* qfp, uint32_t* cnt, uint32_t num, VkQueueRequest* req, uint32_t* fam)
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

void vk_CreateAndInitInstance(VkContext* vk, const Options* opts)
{
	TEST(sysLoadLibrary(VK_LIBRARY, &vk->dll), "ERROR: %s", "Failed to load library");
	memStackFramePush(vk->mem);
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
		char** layers = memStackAlloc(vk->mem, createInfo.enabledLayerCount * sizeof(const char*));
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
		char** ext = memStackAlloc(vk->mem, createInfo.enabledExtensionCount * sizeof(const char*));
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
	memStackFramePop(vk->mem);
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

void vk_GetGraphicsAdapter(VkContext* vk)
{
	memStackFramePush(vk->mem);
	vk->phdev = VK_NULL_HANDLE;
	uint32_t num = 0, idx = INV_IDX, fallback = INV_IDX;
	if (vk->EnumeratePhysicalDevicesImpl(vk->inst, &num, NULL) == VK_SUCCESS && num)
	{
		VkPhysicalDevice* adapters = memStackAlloc(vk->mem, sizeof(VkPhysicalDevice) * num);
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
	memStackFramePop(vk->mem);
	vk->phdMask = 1 << idx;
}

void vk_CreateDeviceAndSwapchain(VkContext* vk, const uint32_t* queueCount, uint32_t numFamilies)
{
	uint32_t numQueues = 0;
	memStackFramePush(vk->mem);
	VkDeviceCreateInfo info = {0};
	VkDeviceQueueCreateInfo* qInfo = memStackAlloc(vk->mem, numFamilies * sizeof(VkDeviceQueueCreateInfo));
	for (uint32_t i = 0; i < numFamilies; i++)
	{
		if (queueCount[i] > 0)
		{
			qInfo[i].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			qInfo[i].pNext = NULL;
			qInfo[i].flags = 0;
			qInfo[i].queueFamilyIndex = i;
			qInfo[i].queueCount = queueCount[i];
			float* prios = memStackAlloc(vk->mem, queueCount[i] * sizeof(float));
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
	vk->fbImage = memForwdAlloc(vk->mem, vk->scSize * sizeof(VkImage));
	vk->fbView = memForwdAlloc(vk->mem, vk->scSize * sizeof(VkImageView));
	vk->frmFbOk = memForwdAlloc(vk->mem, vk->scSize * sizeof(VkSemaphore));
	vk->frmFinished = memForwdAlloc(vk->mem, vk->scSize * sizeof(VkSemaphore));
	vk->frmFence = memForwdAlloc(vk->mem, vk->scSize * sizeof(VkFence));
	vk->GetSwapchainImagesKHRImpl(vk->dev, vk->swapchain, &vk->scSize, vk->fbImage);
	for (uint32_t i = 0; i < vk->scSize; i++)
	{
		VkSemaphoreCreateInfo sInfo = {0};
		sInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		VkFenceCreateInfo fInfo = {0};
		fInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
		VKFN(vk->CreateFenceImpl(vk->dev, &fInfo, vk->alloc, &vk->frmFence[i]));
		VKFN(vk->CreateSemaphoreImpl(vk->dev, &sInfo, vk->alloc, &vk->frmFbOk[i]));
		VKFN(vk->CreateSemaphoreImpl(vk->dev, &sInfo, vk->alloc, &vk->frmFinished[i]));
		VkImageViewCreateInfo ivInfo = {0};
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
	memStackFramePop(vk->mem);
}

VkSurfaceFormatKHR vk_GetSwapchainSurfaceFormat(VkContext* vk)
{
	memStackFramePush(vk->mem);
	VkSurfaceFormatKHR retVal =
	{
		VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR
	};
	uint32_t numFormats = 0;
	VKFN(vk->GetPhysicalDeviceSurfaceFormatsKHRImpl(vk->phdev, vk->surf, &numFormats, NULL));
	VkSurfaceFormatKHR* formats = memStackAlloc(vk->mem, numFormats * sizeof(VkSurfaceFormatKHR));
	vk->GetPhysicalDeviceSurfaceFormatsKHRImpl(vk->phdev, vk->surf, &numFormats, formats);
	for (uint32_t i = 0; i < numFormats; i++)
	{
		if (formats[i].format == VK_FORMAT_B8G8R8A8_SRGB)
		{
			retVal = formats[i];
			break;
		}
	}
	memStackFramePop(vk->mem);
	return retVal;
}

VkPresentModeKHR vk_GetSwapchainPresentMode(VkContext* vk)
{
	memStackFramePush(vk->mem);
	uint32_t numModes = 0;
	VkPresentModeKHR retVal = VK_PRESENT_MODE_FIFO_KHR;
	VKFN(vk->GetPhysicalDeviceSurfacePresentModesKHRImpl(vk->phdev, vk->surf, &numModes, NULL));
	VkPresentModeKHR* modes = memStackAlloc(vk->mem, numModes * sizeof(VkPresentModeKHR));
	vk->GetPhysicalDeviceSurfacePresentModesKHRImpl(vk->phdev, vk->surf, &numModes, modes);
	for (uint32_t i = 0; i < numModes; i++)
		if (modes[i] == VK_PRESENT_MODE_MAILBOX_KHR)
		{
			retVal = modes[i];
			break;
		}
	memStackFramePop(vk->mem);
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

struct VkImageViewImpl
{
    uint32_t width, height;
    VkImageView id;
};

struct VkRenderPassImpl
{
    uint32_t numAttachments;
    struct
    {
        uint32_t num;
        uint32_t current;
        uint32_t width;
        uint32_t height;
        VkFramebuffer* id;
    } fb;
    VkRenderPass id;
};

void vk_CreateRenderPass(const VkContext* vk, const VkRenderPassCreateInfo* info, VkRenderPassRef* pass)
{
    VkRenderPassRef tmp = memForwdAlloc(vk->mem, sizeof(struct VkRenderPassImpl));
    VKFN(vk->CreateRenderPassImpl(vk->dev, info, vk->alloc, &tmp->id));
    tmp->numAttachments = info->attachmentCount;
    *pass = tmp;
}

void vk_DestroyRenderPass(const VkContext* vk, VkRenderPassRef* pass)
{
    VkRenderPassRef tmp = *pass;
    vk->DestroyRenderPassImpl(vk->dev, tmp->id, vk->alloc);
    for (uint32_t i = 0; i < tmp->fb.num; i++)
        vk->DestroyFramebufferImpl(vk->dev, tmp->fb.id[i], vk->alloc);
    *pass = NULL;
}

void vk_CmdBeginRenderPass(const VkContext* vk, VkCommandBuffer cb, VkRenderPassRef pass)
{
    VkRenderPassBeginInfo info = {0};
    info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    info.renderPass = pass->id;
    //info.framebuffer = (fb->numFramebuffers != 1) ? fb->ids[vk->frameIdx] : fb->id;
    //fb->current = (fb->current + 1) % fb->numFramebuffers;
    //info.framebuffer = fb->id[fb->current];
    //TODO
}

void vk_InitPassFramebuffer(const VkContext* vk, VkRenderPassRef pass, const VkImageViewRef* views)
{
    pass->fb.num = 1;
    pass->fb.current = 0;
    memStackFramePush(vk->mem);
    ASSERT_Q((views) || (pass->numAttachments == 1));
    uint32_t scImageIndex = INV_IDX, fbWidth = UINT32_MAX, fbHeight = UINT32_MAX;
    VkImageView* copy = memStackAlloc(vk->mem, pass->numAttachments * sizeof(VkImageView));
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
    pass->fb.id = memForwdAlloc(vk->mem, sizeof(VkFramebuffer) * pass->fb.num);
    VkFramebufferCreateInfo info = {0};
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
    memStackFramePop(vk->mem);
}
