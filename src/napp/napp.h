#pragma once

#ifdef __cplusplus
#ifndef _NAPP_WRL_
#define NAPP_API extern "C"
#else
#define NAPP_API
#endif
#else
#define NAPP_API
#endif

#include <stdbool.h>

#include "napp_assert.h"

NAPP_API void NAppSetFullscreen(bool value);
#define napp_set_fullscreen(v) NAppSetFullscreen(v)

NAPP_API void NAppSetViewSize(int width, int height);
#define napp_set_view_size(w,h) NAppSetViewSize(w,h)

NAPP_API void napp_set_startup_proc(void*(*)(void));

NAPP_API void napp_set_main_loop_proc(void(*)(void*));

NAPP_API void napp_set_shutdown_proc(void(*)(void*));
