#pragma once

#include "napp.h"

#define VK_NO_PROTOTYPES
#include <vulkan/vulkan.h>

#define VULKAN_API_DEVICE(f) extern PFN_vk ## f ## vk ## f ;
#include "napp_vkdevice.h"

#define VKTEST(call) do { VkResult VKRESULT = call ; TEST(VKRESULT==VK_SUCCESS);}  while (0)
#define VKINIT(obj,tname) do { memset(&obj, 0, sizeof(obj));(obj).sType = tname; } while (0)

NAPP_API void NAppVkInit();

NAPP_API VkDevice NAppVkCreateDevice(int phDevice, const VkDeviceCreateInfo* info);
