#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

typedef uint32_t HVkCmdBuff;
typedef uint32_t HVkQueue;
typedef uint32_t HVkPipeline;
typedef uint32_t HVkBuffer; // can be distinct object or sub-allocation, distinct object can have or not have committed memory

struct VklEnv;
typedef struct VklEnv* HVulkan;
    
#ifdef __cplusplus
}
#endif
