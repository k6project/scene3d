#pragma once

class Scene;

typedef void* Mesh;
typedef void* Overlay;
typedef void* Material;

struct IMaterial
{};

struct IMaterialInfo
{
    virtual void SetVertexShader(const char* name) = 0;
    virtual void SetPixelShader(const char* name) = 0;
    virtual void SetBackfaceCulling(bool value) = 0;
};

class IRenderer
{
public:
	static IRenderer* Get();
	virtual void Initialize(void* window) = 0;
	virtual void RenderScene(const Scene* scene) = 0;
    virtual IMaterialInfo* NewMaterialInfo() = 0;
    virtual IMaterial* CreateMaterial(const IMaterialInfo* info) = 0;
	virtual void Finalize() = 0;
};
