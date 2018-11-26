#pragma once

#ifdef _MSC_VER
#define VK_USE_PLATFORM_WIN32_KHR
#else
#define VK_USE_PLATFORM_MACOS_MVK
#endif

#define VK_NO_PROTOTYPES
#include <vulkan/vulkan.h>

#define VK_MAX_BARRIERS_PER_CALL 4

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct
{
	uint32_t family;
	uint32_t index;
	VkQueue queue;
} VkQueueInfo;

typedef struct
{
	PFN_vkGetInstanceProcAddr GetInstanceProcAddrImpl;
#define VULKAN_API_GOBAL(proc) PFN_vk ## proc proc ## Impl;
#define VULKAN_API_INSTANCE(proc) PFN_vk ## proc proc ## Impl;
#define VULKAN_API_DEVICE(proc) PFN_vk ## proc proc ## Impl;
#include "vk_api.inl"
	HMemAlloc mem;
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
} VkContext;
    
typedef struct
{
    VkCommandPool pool;
    VkCommandBuffer* buffers;
} VkCommandRecorder;

typedef struct
{
	VkQueueFlags flags;
	bool present;
} VkQueueRequest;

typedef struct
{
	const VkContext* parent;
	const struct Options* options;
	uint32_t numQueueReq;
	VkQueueRequest* queueReq;
} VkContextInfo;

typedef struct VkImageViewImpl* VkImageViewRef;
typedef struct VkRenderPassImpl* VkRenderPassRef;
    
typedef struct
{
    uint32_t index;
	uint32_t imgIdx;
	VkImage fbImage;
	VkSemaphore fbOk;
	VkSemaphore finished;
	VkFence fence;
	uint32_t numImgBarrier;
	VkImageMemoryBarrier imgBarrier[VK_MAX_BARRIERS_PER_CALL];
	uint32_t numBufBarrier;
	VkBufferMemoryBarrier bufBarrier[VK_MAX_BARRIERS_PER_CALL];
	uint32_t numMemBarrier;
	VkMemoryBarrier memBarrier[VK_MAX_BARRIERS_PER_CALL];
} VkFrame;

void vk_CreateContextImpl(VkContext** ctx, const VkContextInfo* info, HMemAlloc mem);
void vk_DestroyContextImpl(VkContext** ctx);
void vk_BeginFrame(VkContext* vk, VkFrame* frm);
void vk_EndFrame(VkContext* vk, VkFrame* frm);
void vk_InitCommandRecorder(VkContext* vk, VkCommandRecorder* cr, uint32_t queueIdx);
void vk_DestroyCommandRecorder(VkContext* vk, VkCommandRecorder* cr);

void vk_CreateRenderPass(const VkContext* vk, const VkRenderPassCreateInfo* info, VkRenderPassRef* pass);
void vk_InitPassFramebuffer(const VkContext* vk, VkRenderPassRef pass, const VkImageViewRef* views);
void vk_DestroyRenderPass(const VkContext* vk, VkRenderPassRef* pass);
void vk_CmdBeginRenderPass(const VkContext* vk, VkCommandBuffer cb, VkRenderPassRef pass);

#define vk_GetDisplayFormat(v) (v->surfFmt.format)

#define VKFN(c) TEST_Q( (c) == VK_SUCCESS ) 
#define vk_CreateContext(v,o,m) vk_CreateContextImpl(&v,o,m)
#define vk_CreateCommandPool(v,inf,ptr) VKFN((v)->CreateCommandPoolImpl((v)->dev,(inf),(v)->alloc,(ptr)))

#define vk_CreateSemaphore(v,inf,ptr) do { \
	VkSemaphoreCreateInfo info = {0}; \
	info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO; \
    VKFN((v)->CreateSemaphoreImpl((v)->dev,(inf),(v)->alloc,(ptr))) \
} while (0)

#define vk_CreateFence(v,signaled,ptr) do { \
	VkFenceCreateInfo info = {0}; \
	info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO; \
	info.flags = ((signaled)) ? VK_FENCE_CREATE_SIGNALED_BIT : 0; \
	VKFN((v)->CreateFenceImpl((v)->dev,&info,(v)->alloc,(ptr))); \
} while (0)

#define vk_CreateComputePipelines(v,cnt,inf,ptr) VKFN((v)->CreateComputePipelines((v)->dev,(v)->plCache,(cnt),(inf),(v)->alloc,(ptr)))
#define vk_DeviceWaitIdle(v) (v)->DeviceWaitIdleImpl((v)->dev)
#define vk_DestroyContext(v) vk_DestroyContextImpl(&v)

#ifdef __cplusplus
}
#endif
