#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct SceneImpl* Scene;

typedef struct RendererImpl* Renderer;

Renderer GfxCreateRenderer(MemAlloc mem, Options opts);

void GfxRenderFrame(Renderer rnd, Scene scene);

void GfxDestroyRenderer(Renderer rnd);

#ifdef __cplusplus
}
#endif
