#pragma once

#ifdef _NAPP_WINAPI_
#define VK_USE_PLATFORM_WIN32_KHR
#endif

#ifdef _NAPP_MACOS_
#define VK_USE_PLATFORM_MACOS_MVK
#endif

#define VK_NO_PROTOTYPES
#include <vulkan/vulkan.h>
