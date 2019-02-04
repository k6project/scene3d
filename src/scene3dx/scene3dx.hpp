#pragma once

#include <common.hpp>

class RendererAPI;

class Scene3DXApp : Scene
{
public:
	virtual void CommitParameters(void* buffer, size_t max) const override;
	virtual const ScenePrimitive* GetPrimitives() const override;
	bool ShouldKeepRunning() const;
	void Initialize(void* window);
	void Update(float deltaT);
	void Finalize();
private:
	void CreateTextures();
	void CreateMaterials();
	bool KeepRunning = false;
    MemAllocLinear Parameters;
	RendererAPI* Renderer;
};

