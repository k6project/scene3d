#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct RendererImpl* Renderer;

Renderer rnd_CreateRenderer(MemAlloc mem, Options opts);

void rnd_DestroyRenderer(Renderer rnd);

void rnd_RenderFrame(Renderer rnd); // pass scene pointer

//create mesh
//create object (mesh instance): allocate paged buffer region for uniforms

#ifdef __cplusplus
}
#endif
