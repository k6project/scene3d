#pragma once

#include <cstdint>

#define VK_NO_PROTOTYPES
#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>

namespace vk
{
    class Device;
    struct QueueLayout;
    
    class Queue
    {
        friend class QVkDevice;
        uint32_t family_, index_;
        VkQueue id_ = nullptr;
    };

    class Surface
    {
    private:
        friend class QVkDevice;
        friend class QVkInstance;
        void initSwapchainCreateInfo(VkSwapchainCreateInfoKHR& info) const;
        VkSurfaceCapabilitiesKHR capabilities_;
        //QVector<VkSurfaceFormatKHR> formats_;
        //QVector<VkPresentModeKHR> presentModes_;
        VkSurfaceKHR id_ = nullptr;
    };

    class Image
    {
        friend class QVkDevice;
        friend class QVkSwapchain;
        VkImageUsageFlagBits usage_;
        VkSampleCountFlagBits samples_;
        VkFormat format_;
        VkImageView view_;
        VkImage id_;
    };

    class Swapchain
    {
        friend class QVkDevice;
        uint32_t size_ = 0, current_ = 0;
        VkSwapchainKHR id_ = nullptr;
        VkImageView* view_ = nullptr;
        VkImage* image_ = nullptr;
        Image proxy_;
    };

    class Instance
    {
    public:
        static const Instance& get();
        static void destroy();
        void createSurface(Surface& surface, void* view) const;
        void destroySurface(Surface& surface) const;
        void createDevice(Device& device, const Surface& surface, uint32_t numExt = 0, const char **ext = nullptr) const;
        void destroyDevice(Device& device) const;
    private:
        static Instance GInstance;
        void initInstanceFunctions();
        void initDeviceFunctions(Device& device);
        const QueueLayout& queueLayout(uint32_t vendorId, uint32_t deviceId) const;
        #define VULKAN_API_GLOBAL(proc) PFN_vk ## proc vk ## proc = nullptr;
        #define VULKAN_API_INSTANCE(proc) PFN_vk ## proc vk ## proc = nullptr;
        #include "vulkan.inl"
        PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr = nullptr;
        VkDebugReportCallbackEXT debug_ = nullptr;
        VkInstance id_ = nullptr;
        void* library_;
    };

    class Device
    {
    public:
        void createSwapchain(Swapchain& swapchain, Surface& surface);
        bool acquireNextImage(const Swapchain& swapchain) const;
        void queuePresent(const Swapchain& swapchain, const Queue& queue);
        void waitIdle() const;
        const VkPhysicalDeviceProperties& properties() const;
    protected:
        friend class QVkInstance;
        void setQueues(const QueueLayout& qLayout);
        void setSurfaceCapabilities(Surface& surface) const;
        PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR vkGetPhysicalDeviceSurfaceCapabilitiesKHR;
        PFN_vkGetPhysicalDeviceSurfaceFormatsKHR vkGetPhysicalDeviceSurfaceFormatsKHR;
        PFN_vkGetPhysicalDeviceSurfacePresentModesKHR vkGetPhysicalDeviceSurfacePresentModesKHR;
        #define VULKAN_API_DEVICE(proc) PFN_vk ## proc vk ## proc = nullptr;
        #include "vulkan.inl"
        VkPhysicalDeviceProperties properties_;
        VkPhysicalDeviceFeatures features_;
        VkPhysicalDeviceMemoryProperties memoryProperties_;
        VkPhysicalDevice adapter_;
        VkDevice id_ = nullptr;
        Queue drawQueue_;
        Queue copyQueue_;
    };
}
