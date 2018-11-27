#pragma once

typedef struct RendererImpl* HRenderer;

void rnd_CreateRenderer(HMemAlloc mem, const struct Options* opts, HRenderer* rdrPtr);
void rnd_DestroyRenderer(HRenderer rnd);
void rnd_RenderFrame(HRenderer rnd);
