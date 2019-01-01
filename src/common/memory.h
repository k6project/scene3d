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

MemAlloc MemAllocCreate(size_t forwd, size_t stack, void* block, size_t max);

void mem_AllocRelease(MemAlloc mem);

void* mem_ForwdAlloc(MemAlloc mem, size_t bytes);

void* mem_StackAlloc(MemAlloc mem, size_t bytes);

void mem_StackFramePush(MemAlloc mem);

void mem_StackFramePop(MemAlloc mem);

void* mem_HeapAlloc(MemAlloc mem, size_t bytes);

void mem_HeapFree(MemAlloc mem, void* ptr);

size_t mem_SubAllocSize(size_t bytes);

#ifdef __cplusplus
}
#endif
