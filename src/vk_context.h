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

typedef struct VkContextImpl* HVkContext;
typedef struct VkTexture2DImpl* HVkTexture2D;
typedef struct VkRenderPassImpl* HVkRenderPass;
typedef struct VkFrameContextImpl* HFrameContext;
typedef struct VkCommandBufferImpl* VkCommandBuferRef;

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
	HVkContext parent;
	const struct Options* options;
	uint32_t numQueueReq;
	VkQueueRequest* queueReq;
} VkRenderContextInfo;
    
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

void vk_CreateRenderContext(HMemAlloc mem, const VkRenderContextInfo* info, HVkContext* vkPtr);
void vk_DestroyRenderContext(HVkContext vk);
void vk_BeginFrame(HVkContext vk);
VkFormat vk_GetSwapchainImageFormat(HVkContext vk);
VkCommandBuffer vk_GetPrimaryCommandBuffer(HVkContext vk);
void vk_SubmitFrame(HVkContext vk, uint32_t queue);
void vk_CreateRenderPass(HVkContext vk, const VkRenderPassCreateInfo* info, HVkRenderPass* pass);
void vk_InitPassFramebuffer(HVkContext vk, HVkRenderPass pass, const HVkTexture2D* textures);
void vk_DestroyRenderPass(HVkContext vk, HVkRenderPass pass);
void vk_CmdBeginRenderPass(HVkContext vk, VkCommandBuffer cb, HVkRenderPass pass);
void vk_CmdEndRenderPass(HVkContext vk, VkCommandBuffer cb);

#define vk_GetDisplayFormat(v) (v->surfFmt.format)

#define VKFN(c) TEST_Q( (c) == VK_SUCCESS ) 

#ifdef __cplusplus
}
#endif
