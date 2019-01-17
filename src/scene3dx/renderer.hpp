#pragma once

class Scene;

class IRenderer
{
public:
	typedef void* MeshRef;
	typedef void* OverlayRef;
	static IRenderer* Get();
	virtual void Initialize(void* window) = 0;
	virtual void RenderScene(const Scene* scene) = 0;
	virtual OverlayRef CreateOverlay() = 0;
	virtual void Finalize() = 0;
};
