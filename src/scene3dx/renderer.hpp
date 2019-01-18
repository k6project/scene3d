#pragma once

class Scene;

typedef void* MeshRef;
typedef void* OverlayRef;
typedef void* MaterialRef;

struct MaterialInfo
{
	const char* VSName;
	const char* PSName;
};

class IRenderer
{
public:
	static IRenderer* Get();
	virtual void Initialize(void* window) = 0;
	virtual void RenderScene(const Scene* scene) = 0;
	virtual MaterialRef CreateMaterial(const MaterialInfo& info) = 0;
	virtual OverlayRef CreateOverlay(MaterialRef material) = 0;
	virtual void Finalize() = 0;
};
