#pragma once

#include "memory.hpp"
#include "vecmath.hpp"
#include "renderer.hpp"

class TileMap
{

public:

	void Initialize(uint32_t rows, uint32_t cols, const char* data = nullptr);

	void SetTileData(const char* data);

	void Finalize();

private:

	struct Primitive : public ScenePrimitive
	{
	};

	static void CommitParameters(void* ptr, size_t max);

	MemAllocLinear Primitives;
	
	Material* MapMaterial;

	Vec2u GridSize;

};
