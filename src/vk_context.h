#pragma once

#ifdef _MSC_VER
#define VK_USE_PLATFORM_WIN32_KHR
#else
#define VK_USE_PLATFORM_MACOS_MVK
#endif

#define VK_NO_PROTOTYPES
#include <vulkan/vulkan.h>

#ifdef __cplusplus
extern "C"
{
#endif

struct Options;

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
	VkDevice dev;
	VkPipelineCache plCache;
	VkQueueInfo* queues;
	VkSwapchainKHR swapchain;
	VkImage* fbImage; // array holding swapchain images
	VkSemaphore* frmFbOk; // semaphores to signify image acquisition
	VkSemaphore* frmFinished; // semaphores to signal to present
	VkFence* frmFence;
	uint32_t frameIdx; // current frame index
} VkContext;
    
//Thread-local: command pool + commandbuffers
//Begin frame allocates array of command buffers (depending on renderer architecture)
//them functions are called (runCompute(<command buffer slot>), drawEverything(<...>))
//internally, each runs on other thread and fills the slot with command buffer

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

typedef struct
{
    uint32_t index;
	uint32_t imgIdx;
	VkImage fbImage;
	VkSemaphore fbOk;
	VkSemaphore finished;
	VkFence fence;
} VkFrame;

void vk_CreateContextImpl(VkContext** ctx, const VkContextInfo* info, HMemAlloc mem);
void vk_DestroyContextImpl(VkContext** ctx);
void vk_BeginFrame(VkContext* vk, VkFrame* frm);
void vk_EndFrame(VkContext* vk, VkFrame* frm);
void vk_InitCommandRecorder(VkContext* vk, VkCommandRecorder* cr, uint32_t queueIdx);
void vk_DestroyCommandRecorder(VkContext* vk, VkCommandRecorder* cr);
    
#define vk_GetCommandBuffer(f,cr) ((cr).buffers[(f).index])

#define VKFN(c) TEST_Q( (c) == VK_SUCCESS ) 
#define vk_CreateContext(v,o,m) vk_CreateContextImpl(&v,o,m)
#define vk_CreateCommandPool(v,inf,ptr) VKFN((v)->CreateCommandPoolImpl((v)->dev,(inf),(v)->alloc,(ptr)))
	//CreateCommandBuffer ???

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

//CreateDescriptorPool
#define vk_CreateComputePipelines(v,cnt,inf,ptr) VKFN((v)->CreateComputePipelines((v)->dev,(v)->plCache,(cnt),(inf),(v)->alloc,(ptr)))
#define vk_DeviceWaitIdle(v) (v)->DeviceWaitIdleImpl((v)->dev)
#define vk_DestroyContext(v) vk_DestroyContextImpl(&v)

#ifdef __cplusplus
}
#endif
