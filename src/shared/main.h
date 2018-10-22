#pragma once

#include <stddef.h>
#include <stdbool.h>

#include "args.h"

#ifdef __cplusplus
extern "C"
{
#endif
    
typedef struct AppCallbacks
{
    void (*beforeStart)(void*);
    void (*beforeStop)(void*);
} AppCallbacks;

void appInitialize(AppCallbacks* callbacks, void* state);
    
bool appShouldKeepRunning(void);
    
void appPollEvents(void);

bool appLoadLibrary(const TChar* name, void** handle);

void* appGetLibraryProc(void* handle, const char* name);

void appUnloadLibrary(void* handle);

void appTCharToUTF8(char* dest, const TChar* src, int max);
    
void appPrintf(const TChar* fmt, ...);

#ifdef __cplusplus
}
#endif
