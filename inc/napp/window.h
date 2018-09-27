#pragma once

#include "macros.h"

#ifndef __cplusplus
#include <stdbool.h>
#endif

NAPP_API void napp_set_fullscreen(bool value);

NAPP_API void napp_set_window_size(int width, int height);
