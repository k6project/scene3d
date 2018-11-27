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

MemAlloc memAllocCreate(size_t forwd, size_t stack, void* block, size_t max)
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

void* memForwdAlloc(MemAlloc mem, size_t bytes)
{
	bytes = ALIGN16(bytes);
	ASSERT_Q(mem->maxForwd - mem->posForwd >= bytes);
	void* retval = mem->baseForwd + mem->posForwd;
	mem->posForwd += bytes;
	return retval;
}

void* memStackAlloc(MemAlloc mem, size_t bytes)
{
	bytes = ALIGN16(bytes);
	ASSERT_Q(mem->currFrame < mem->maxStack || mem->posStack == 0);
	ASSERT_Q(mem->maxStack - mem->posStack >= bytes);
	void* retval = mem->baseStack + mem->posStack;
	mem->posStack += bytes;
	return retval;
}

void memStackFramePush(MemAlloc mem)
{
	size_t pos = mem->posStack;
	size_t* prev = memStackAlloc(mem, sizeof(size_t));
	*prev = mem->currFrame;
	mem->currFrame = pos;
}

void memStackFramePop(MemAlloc mem)
{
	ASSERT_Q(mem->currFrame < mem->maxStack);
    mem->posStack = mem->currFrame;
    mem->currFrame = *(size_t*)(mem->baseStack + mem->currFrame);
}

void memAllocRelease(MemAlloc mem)
{
	void* block = mem->memBlock;
	free(block);
}

void* memHeapAlloc(MemAlloc mem, size_t bytes)
{
	return malloc(bytes);
}

void memHeapFree(MemAlloc mem, void* ptr)
{
	free(ptr);
}

size_t memSubAllocSize(size_t bytes)
{
	return (bytes + sizeof(struct MemAllocImpl));
}
