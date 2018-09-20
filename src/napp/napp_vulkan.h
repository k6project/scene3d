#pragma once

#include "napp.h"

#define VK_NO_PROTOTYPES
#include <vulkan/vulkan.h>

#define VULKAN_API_DEVICE(f) extern PFN_vk ## f ## vk ## f ;
#include "napp_vkdevice.h"

NAPP_API void NAppVkInit();

NAPP_API VkDevice NAppVkCreateDevice(int phDevice, const VkDeviceCreateInfo* info);