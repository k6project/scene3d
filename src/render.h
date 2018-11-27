#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct RendererImpl* Renderer;

void rnd_CreateRenderer(MemAlloc mem, const struct Options* opts, Renderer* rdrPtr);
void rnd_DestroyRenderer(Renderer rnd);
void rnd_RenderFrame(Renderer rnd);

#ifdef __cplusplus
}
#endif
