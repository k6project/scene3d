#pragma once

#include "global.h"
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

#ifdef __cplusplus
}
#endif
