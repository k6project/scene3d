#pragma once

#include "main.h"

#ifndef NO_VULKAN
#include "vk_api.c"
#endif

int appMain(int argc, const TChar** argv);

#ifdef _MSC_VER

#include <windows.h>

#include "win32.c"

int CALLBACK WinMain(HINSTANCE inst, HINSTANCE prev, LPSTR cmd, int show)
{
	int argc = 0;
	const TChar** argv = (const TChar**)CommandLineToArgvW(GetCommandLine(), &argc);
    appMain(argc, argv);
	LocalFree((HLOCAL)argv);
	return show;
}

#else // OSX

int main(int argc, const TChar** argv)
{
    int retval = appMain(argc, argv);
    return retval;
}

#endif

struct MemAlloc
{
    void *memBlock;
    uint8_t* baseForwd;
    size_t posForwd, maxForwd;
    uint8_t* baseStack;
    size_t posStack, maxStack, currFrame;
};

HMemAlloc memAllocCreate(size_t forwd, size_t stack, void* block, size_t max)
{
    forwd = ALIGN16(forwd);
    stack = ALIGN16(stack);
    size_t offset = ALIGN16(sizeof(struct MemAlloc));
    size_t total = offset + forwd + stack;
    struct MemAlloc* retval = block;
    if (retval == NULL)
    {
        ASSERT_Q(retval = malloc(total));
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
/*
 push: write value of current frame into stack, store its offset as new value for current frame
 alloc: check if stack frame is valid
 pop if currentFrame < maxStack
 */

void memAllocRelease(HMemAlloc mem)
{
    void* block = mem->memBlock;
    free(block);
}
