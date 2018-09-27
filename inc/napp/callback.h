#pragma once

#include "macros.h"

enum
{
    NAPP_STARTUP,
    NAPP_UPDATE_BEGIN,
    NAPP_UPDATE_END,
    NAPP_SHUTDOWN,
    NAPP_CALLBACKS
};

typedef void(*napp_callback_t)(void*);

NAPP_API void napp_set_context(void* context);

NAPP_API void napp_set_callback(unsigned int index, napp_callback_t proc);

#ifdef __cplusplus
template <typename T, void(T::*M)()>
void napp_cb(void* o) { (static_cast<T*>(o)->*M)(); }
#endif
