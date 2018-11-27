#pragma once

#ifdef __cplusplus
extern "C"
{
#endif
    
/* Memory allocation modes */
typedef enum
{
    MEM_FORWD,
    MEM_STACK,
    MEM_HEAP
} MemAllocMode;

typedef struct MemAllocImpl* MemAlloc;

MemAlloc memAllocCreate(size_t forwd, size_t stack, void* block, size_t max);
void memAllocRelease(MemAlloc mem);
void* memForwdAlloc(MemAlloc mem, size_t bytes);
void* memStackAlloc(MemAlloc mem, size_t bytes);
void memStackFramePush(MemAlloc mem);
void memStackFramePop(MemAlloc mem);
void* memHeapAlloc(MemAlloc mem, size_t bytes);
void memHeapFree(MemAlloc mem, void* ptr);
size_t memSubAllocSize(size_t bytes);

#ifdef __cplusplus
}
#endif
