#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

struct MemAlloc;
typedef struct MemAlloc* HMemAlloc;
HMemAlloc memAllocCreate(size_t forwd, size_t stack, void* block, size_t max);
void memAllocRelease(HMemAlloc mem);
void* memForwdAlloc(HMemAlloc mem, size_t bytes);
void* memStackAlloc(HMemAlloc mem, size_t bytes);
void memStackFramePush(HMemAlloc mem);
void memStackFramePop(HMemAlloc mem);
void* memHeapAlloc(HMemAlloc mem, size_t bytes);
void memHeapFree(HMemAlloc mem, void* ptr);

#ifdef __cplusplus
}
#endif
