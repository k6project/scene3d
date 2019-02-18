#include "tilemap.hpp"

void TileMap::Initialize(uint32_t rows, uint32_t cols, const char* data /*= nullptr*/)
{
	MaterialDescriptor mInfo = {};
	mInfo.VertexShader.LoadFromFile("TileMapVertexShader.cso");
	mInfo.PixelShader.LoadFromFile("TileMapPixelShader.cso");
	RendererAPI::Get()->CreateMaterial(mInfo, &MapMaterial);
	Primitives.Init(rows * cols * 5 * sizeof(TileMap::Primitive));
	GridSize.r = rows;
	GridSize.c = cols;
	SetTileData(data);
}

void TileMap::SetTileData(const char* data)
{
	if (data)
	{
		for (const char* ptr = data; *ptr; ptr++)
		{
			switch (*ptr)
			{
			case '.': // FLOOR
			case '#': // WALL
			default:
				break;
			}
		}
	}
}

void TileMap::Finalize()
{
	Primitives.Destroy();
}
