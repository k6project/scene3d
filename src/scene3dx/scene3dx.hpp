#pragma once

#include <common.hpp>
#include <vecmath.hpp>

class RendererAPI;
struct ScenePrimitive;

class Scene3DXApp : Scene
{
public:
	explicit Scene3DXApp();
	virtual void CommitParameters(void* buffer, size_t max) const override;
	virtual const ScenePrimitive* GetPrimitives() const override;
	bool ShouldKeepRunning() const;
	void Initialize(void* window);
	void Update(float deltaT);
	void Finalize();
protected:
	float VerticalFOV, ClipDistance;
	Vec3f CameraPosition, ViewDirection;
	Vec3f UpVector, RightVector;
private:
	const ScenePrimitive* Primitives = nullptr;
	void CreateTextures();
	void CreateMaterials();
	bool KeepRunning = false;
    MemAllocLinear Parameters;
	RendererAPI* Renderer;
};

