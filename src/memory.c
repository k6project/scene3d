#include "global.h"

#include "memory.h"

#include <stdlib.h>

struct MemAllocImpl
{
	void *memBlock;
	uint8_t* baseForwd;
	size_t posForwd, maxForwd;
	uint8_t* baseStack;
	size_t posStack, maxStack, currFrame;
};

MemAlloc mem_AllocCreate(size_t forwd, size_t stack, void* block, size_t max)
{
	forwd = ALIGN16(forwd);
	stack = ALIGN16(stack);
	size_t offset = ALIGN16(sizeof(struct MemAllocImpl));
	size_t total = offset + forwd + stack;
	struct MemAllocImpl* retval = block;
	if (retval == NULL)
	{
		TEST_Q(retval = malloc(total));
		retval->memBlock = retval;
		max = total;
	}
	else
		retval->memBlock = NULL;
	ASSERT_Q(total <= max);
	retval->baseForwd = ((uint8_t*)retval) + offset;
	retval->posForwd = 0;
	retval->maxForwd = forwd;
	retval->baseStack = retval->baseForwd + forwd;
	retval->maxStack = stack;
	retval->posStack = 0;
	retval->currFrame = stack;
	return retval;
}

void* mem_ForwdAlloc(MemAlloc mem, size_t bytes)
{
	bytes = ALIGN16(bytes);
	ASSERT_Q(mem->maxForwd - mem->posForwd >= bytes);
	void* retval = mem->baseForwd + mem->posForwd;
	mem->posForwd += bytes;
	return retval;
}

void* mem_StackAlloc(MemAlloc mem, size_t bytes)
{
	bytes = ALIGN16(bytes);
	ASSERT_Q(mem->currFrame < mem->maxStack || mem->posStack == 0);
	ASSERT_Q(mem->maxStack - mem->posStack >= bytes);
	void* retval = mem->baseStack + mem->posStack;
	mem->posStack += bytes;
	return retval;
}

void mem_StackFramePush(MemAlloc mem)
{
	size_t pos = mem->posStack;
	size_t* prev = mem_StackAlloc(mem, sizeof(size_t));
	*prev = mem->currFrame;
	mem->currFrame = pos;
}

void mem_StackFramePop(MemAlloc mem)
{
	ASSERT_Q(mem->currFrame < mem->maxStack);
    mem->posStack = mem->currFrame;
    mem->currFrame = *(size_t*)(mem->baseStack + mem->currFrame);
}

void mem_AllocRelease(MemAlloc mem)
{
	void* block = mem->memBlock;
	free(block);
}

void* mem_HeapAlloc(MemAlloc mem, size_t bytes)
{
	return malloc(bytes);
}

void mem_HeapFree(MemAlloc mem, void* ptr)
{
	free(ptr);
}

size_t mem_SubAllocSize(size_t bytes)
{
	return (bytes + sizeof(struct MemAllocImpl));
}
