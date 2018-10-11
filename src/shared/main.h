#pragma once

#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C"
{
#endif
    
typedef struct AppCallbacks
{
    void (*beforeStart)(void*);
} AppCallbacks;

void appInitialize(AppCallbacks* callbacks, void* state);
    
bool appShouldKeepRunning(void);
    
void appPollEvents(void);
    
#ifdef __cplusplus
}
#endif
