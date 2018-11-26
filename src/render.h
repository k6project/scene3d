#pragma once

#include "vk_context.h"

typedef struct
{
    HMemAlloc mem, outer;
    VkContext* vulkan;
    VkCommandRecorder cmdRec;
    VkRenderPassRef basePass;
} Renderer;

void rdr_CreateRenderer(HMemAlloc mem, const struct Options* opts, Renderer** rdrPtr);
void rdr_DestroyRenderer(Renderer** rdrPtr);
