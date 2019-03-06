#pragma once

#include <common.hpp>
#include <isoview.hpp>
#include <tilemap.hpp>

class RendererAPI;
struct ScenePrimitive;

class Scene3DXApp : Scene
{
public:
	explicit Scene3DXApp();
	virtual void CommitParameters(void* buffer, size_t max) const override;
	virtual const ScenePrimitive* GetPrimitives() const override;
	bool ShouldKeepRunning() const;
	void Initialize(void* window, uint32_t w, uint32_t h);
	void MouseMoved(int32_t x, int32_t y, bool lb);
	void Update(float deltaT);
	void Finalize();
protected:
	float VerticalFOV, ClipDistance;
	Vec3f CameraPosition, ViewDirection;
	Vec3f UpVector, RightVector, ViewOrigin;
	Vec4f ViewportDimensions;
	union 
	{ 
		struct { float Val, Rcp; };
		Vec2f Vector;
	} AspectRate;
	struct
	{
		bool Captured = false;
		struct { int32_t x, y; } Pos;
	} Mouse;
private:
	ScenePrimitive* Primitives = nullptr;
	void CreateTextures();
	void CreateMaterials();
	bool KeepRunning = false;
    MemAllocLinear Parameters;
	RendererAPI* Renderer;
	TileMap LevelMap;
    IsoView MapView;
};

