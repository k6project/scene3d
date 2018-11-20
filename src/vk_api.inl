#ifndef VULKAN_API_GOBAL
#define VULKAN_API_GOBAL(proc)
#endif
VULKAN_API_GOBAL(CreateInstance)
VULKAN_API_GOBAL(EnumerateInstanceLayerProperties)
VULKAN_API_GOBAL(EnumerateInstanceExtensionProperties)
#undef VULKAN_API_GOBAL

#ifndef VULKAN_API_INSTANCE
#define VULKAN_API_INSTANCE(proc)
#endif
VULKAN_API_INSTANCE(DestroyInstance)
VULKAN_API_INSTANCE(EnumeratePhysicalDevices)
VULKAN_API_INSTANCE(EnumerateDeviceExtensionProperties)
VULKAN_API_INSTANCE(GetPhysicalDeviceProperties)
VULKAN_API_INSTANCE(GetPhysicalDeviceFeatures)
VULKAN_API_INSTANCE(GetPhysicalDeviceMemoryProperties)
VULKAN_API_INSTANCE(GetPhysicalDeviceQueueFamilyProperties)
VULKAN_API_INSTANCE(GetPhysicalDeviceSurfaceSupportKHR)
VULKAN_API_INSTANCE(GetPhysicalDeviceSurfaceCapabilitiesKHR)
VULKAN_API_INSTANCE(GetPhysicalDeviceSurfaceFormatsKHR)
VULKAN_API_INSTANCE(GetPhysicalDeviceSurfacePresentModesKHR)
VULKAN_API_INSTANCE(DestroySurfaceKHR)
#if defined(VK_USE_PLATFORM_WIN32_KHR)
VULKAN_API_INSTANCE(CreateWin32SurfaceKHR)
#elif defined(VK_USE_PLATFORM_MACOS_MVK)
VULKAN_API_INSTANCE(CreateMacOSSurfaceMVK)
#endif
#ifdef _DEBUG
VULKAN_API_INSTANCE(CreateDebugReportCallbackEXT)
VULKAN_API_INSTANCE(DebugReportMessageEXT)
VULKAN_API_INSTANCE(DestroyDebugReportCallbackEXT)
#endif
VULKAN_API_INSTANCE(GetDeviceProcAddr)
VULKAN_API_INSTANCE(CreateDevice)
#undef VULKAN_API_INSTANCE

#ifndef VULKAN_API_DEVICE
#define VULKAN_API_DEVICE(proc)
#endif
VULKAN_API_DEVICE(GetDeviceQueue)
VULKAN_API_DEVICE(CreateSwapchainKHR)
VULKAN_API_DEVICE(DestroySwapchainKHR)
VULKAN_API_DEVICE(CreateFramebuffer)
VULKAN_API_DEVICE(DestroyFramebuffer)
VULKAN_API_DEVICE(GetSwapchainImagesKHR)
VULKAN_API_DEVICE(CreateImageView)
VULKAN_API_DEVICE(DestroyImageView)
VULKAN_API_DEVICE(CreateCommandPool)
VULKAN_API_DEVICE(DestroyCommandPool)
VULKAN_API_DEVICE(AllocateCommandBuffers)
VULKAN_API_DEVICE(FreeCommandBuffers)
VULKAN_API_DEVICE(CreateRenderPass)
VULKAN_API_DEVICE(DestroyRenderPass)
#ifdef _MSC_VER
#undef CreateSemaphore
VULKAN_API_DEVICE(CreateSemaphore)
#ifdef UNICODE
#define CreateSemaphore  CreateSemaphoreW
#else
#define CreateSemaphore  CreateSemaphoreA
#endif
#else
VULKAN_API_DEVICE(CreateSemaphore)
#endif
VULKAN_API_DEVICE(DestroySemaphore)
VULKAN_API_DEVICE(CreateFence)
VULKAN_API_DEVICE(DestroyFence)
VULKAN_API_DEVICE(WaitForFences)
VULKAN_API_DEVICE(ResetFences)
VULKAN_API_DEVICE(CreateShaderModule)
VULKAN_API_DEVICE(DestroyShaderModule)
VULKAN_API_DEVICE(CreateDescriptorSetLayout)
VULKAN_API_DEVICE(DestroyDescriptorSetLayout)
VULKAN_API_DEVICE(CreateDescriptorPool)
VULKAN_API_DEVICE(DestroyDescriptorPool)
VULKAN_API_DEVICE(AllocateDescriptorSets)
VULKAN_API_DEVICE(FreeDescriptorSets)
VULKAN_API_DEVICE(UpdateDescriptorSets)
VULKAN_API_DEVICE(CreatePipelineLayout)
VULKAN_API_DEVICE(DestroyPipelineLayout)
VULKAN_API_DEVICE(CreateGraphicsPipelines)
VULKAN_API_DEVICE(CreateComputePipelines)
VULKAN_API_DEVICE(DestroyPipeline)
VULKAN_API_DEVICE(CreateBuffer)
VULKAN_API_DEVICE(DestroyBuffer)
VULKAN_API_DEVICE(CreateImage)
VULKAN_API_DEVICE(DestroyImage)
VULKAN_API_DEVICE(GetBufferMemoryRequirements)
VULKAN_API_DEVICE(GetImageMemoryRequirements)
VULKAN_API_DEVICE(AllocateMemory)
VULKAN_API_DEVICE(BindBufferMemory)
VULKAN_API_DEVICE(BindImageMemory)
VULKAN_API_DEVICE(MapMemory)
VULKAN_API_DEVICE(UnmapMemory)
VULKAN_API_DEVICE(FreeMemory)
VULKAN_API_DEVICE(AcquireNextImageKHR)
VULKAN_API_DEVICE(AcquireNextImage2KHR)
VULKAN_API_DEVICE(BeginCommandBuffer)
VULKAN_API_DEVICE(EndCommandBuffer)
VULKAN_API_DEVICE(ResetCommandBuffer)
VULKAN_API_DEVICE(CmdBeginRenderPass)
VULKAN_API_DEVICE(CmdEndRenderPass)
VULKAN_API_DEVICE(CmdBindPipeline)
VULKAN_API_DEVICE(CmdBindDescriptorSets)
VULKAN_API_DEVICE(CmdPushConstants)
VULKAN_API_DEVICE(CmdDispatch)
VULKAN_API_DEVICE(CmdDraw)
VULKAN_API_DEVICE(CmdPipelineBarrier)
VULKAN_API_DEVICE(CmdClearColorImage)
VULKAN_API_DEVICE(CmdCopyBufferToImage)
VULKAN_API_DEVICE(CmdBlitImage)
VULKAN_API_DEVICE(QueueSubmit)
VULKAN_API_DEVICE(QueueWaitIdle)
VULKAN_API_DEVICE(QueuePresentKHR)
VULKAN_API_DEVICE(DeviceWaitIdle)
VULKAN_API_DEVICE(DestroyDevice)
#undef VULKAN_API_DEVICE
