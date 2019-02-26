#pragma once

#include "memory.hpp"
#include "vecmath.hpp"
#include "renderer.hpp"

struct TileInfo;

struct TilePrimitive;

class TileMap
{

public:

	TileMap();

	void Initialize(uint32_t rows, uint32_t cols, const char* data = nullptr);

	void LoadFromTMX(const char* fileName);

	void SetTileData(const char* data);

	ScenePrimitive* GetPrimitives();

	void Finalize();

private:

	void CollectPrimitives();

	void AddTileFace(float rx, float ry, float rz, float dz);

	static void CommitParameters(ScenePrimitive* prim, void* ptr, size_t max);

	MemAllocLinear TileMapMem;
	
	TilePrimitive* Primitives;

	Material* MapMaterial;

	TileInfo* Tiles;

	bool IsChanged;

	Vec2u GridSize;

};
