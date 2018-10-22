#pragma once

#include <stdbool.h>

#define VK_NO_PROTOTYPES
#include <vulkan/vulkan.h>

#ifdef __cplusplus
extern "C"
{
#endif

#define VULKAN_API_GOBAL(proc) extern PFN_vk ## proc vk ## proc;
#define VULKAN_API_INSTANCE(proc) extern PFN_vk ## proc vk ## proc;
#define VULKAN_API_DEVICE(proc) extern PFN_vk ## proc vk ## proc;
#include "vk_api.inl"

#define VK_INIT(v,t) do{memset(&v,0,sizeof(v));v.sType=t;}while(0)
#define VERIFY(c,r,m) do{if(!(c)){appPrintf(m"\n");return r;}}while(0)
#define VERIFY_VOID(c,m) do{if(!(c)){appPrintf(m"\n");return;}}while(0)
#define VERIFY_NO_MSG(c,r,m) do{if(!(c)){return r;}}while(0)
#define VERIFY_VOID_NO_MSG(c) do{if(!(c)){return;}}while(0)

bool vkCreateAndInitInstanceAPP(void* dll, const VkAllocationCallbacks* alloc, VkInstance* inst);

bool vkCreateSurfaceAPP(VkInstance inst, const VkAllocationCallbacks* alloc, VkSurfaceKHR* surface);
    
bool vkGetAdapterAPP(VkInstance inst, VkSurfaceKHR surface, VkPhysicalDevice* adapter);
    
#ifdef __cplusplus
}
#endif
