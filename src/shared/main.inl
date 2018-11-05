#pragma once

#include "main.h"

#include "vk_api.c"

int appMain(int argc, const TChar** argv);

#ifdef _MSC_VER

#include <windows.h>

#include "win32.c"

int CALLBACK WinMain(HINSTANCE inst, HINSTANCE prev, LPSTR cmd, int show)
{
	int argc = 0;
	const TChar** argv = CommandLineToArgvW(GetCommandLine(), &argc);
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

#define MEM_FORWD_MIN 65536
#define MEM_STACK_MIN 65536

struct MemAlloc
{
    uint8_t* baseForwd;
    size_t posForwd, maxForwd;
    uint8_t* baseStack;
    size_t posStack, maxStack;
};

MemAlloc memAllocCreate(size_t forwd, size_t stack)
{
    forwd = ALIGN16(forwd);
    forwd = (forwd < MEM_FORWD_MIN) ? MEM_FORWD_MIN : forwd;
    stack = ALIGN16(stack);
    stack = (stack < MEM_STACK_MIN) ? MEM_STACK_MIN : stack;
    size_t offset = ALIGN16(sizeof(struct MemAlloc));
    MemAlloc retval = malloc(offset + forwd + stack);
    retval->baseForwd = ((uint8_t*)retval) + offset;
    retval->maxForwd = forwd;
    retval->baseStack = retval->baseForwd + forwd;
    retval->maxStack = stack;
    return retval;
}

void memAllocRelease(MemAlloc mem)
{
    free(mem);
}

#undef MEM_FORWD_MIN
#undef MEM_STACK_MIN
