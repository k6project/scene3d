#include "tilemap.hpp"

#include <tinyxml2.h>

static const float TILE_COLOR[]
{
	PackColor(255, 255, 0, 255), 
	PackColor(255, 0, 0, 255)
};

struct TileInfo
{
	Vec2i Position;
	int32_t Type;
};

struct TileParameters
{
	Mat4f ModelTransform;
	Vec4f FaceNormalAndColor;
};

struct TilePrimitive : public ScenePrimitive
{
	TileInfo* Info = nullptr;
};

TileMap::TileMap()
	: Primitives(nullptr)
	, Tiles(nullptr)
	, IsChanged(false)
{
}

void TileMap::Initialize(uint32_t rows, uint32_t cols, const char* data /*= nullptr*/)
{
	tinyxml2::XMLDocument xmlMap;
	xmlMap.LoadFile("maps/default.tmx");
	if (tinyxml2::XMLElement* xmlRoot = xmlMap.FirstChildElement("map"))
	{
		int xmlWidth = atoi(xmlRoot->Attribute("width"));
		int xmlHeight = atoi(xmlRoot->Attribute("height"));
		if (tinyxml2::XMLElement* xmlLayer = xmlRoot->FirstChildElement("layer"))
		{
			tinyxml2::XMLElement* xmlData = xmlLayer->FirstChildElement("data");
			const char* csvData = xmlData->GetText();
		}
	}
	MaterialDescriptor mInfo = {};
	mInfo.VertexShader.LoadFromFile("TileMapVertexShader.cso");
	mInfo.PixelShader.LoadFromFile("TileMapPixelShader.cso");
	//Renderer_CreateMaterial(mInfo, &MapMaterial);
	RendererAPI::Get()->CreateMaterial(mInfo, &MapMaterial);
	size_t memSize = rows * cols * 5 * sizeof(TilePrimitive);
	memSize += rows * cols * sizeof(TileInfo);
	TileMapMem.Init(memSize);
	GridSize.h = rows;
	GridSize.w = cols;
	SetTileData(data);
}

void TileMap::SetTileData(const char* data)
{
	if (!Tiles)
	{
		Tiles = TileMapMem.TAlloc<TileInfo>(GridSize.w * GridSize.h);
	}
	if (!Primitives)
	{
		Primitives = TileMapMem.TAlloc<TilePrimitive>(GridSize.w * GridSize.h);
	}
	if (data)
	{
		int32_t hw = -static_cast<int32_t>(GridSize.w >> 1);
		int32_t hh = -static_cast<int32_t>(GridSize.h >> 1);
		int32_t x = hw, y = -hh;
		for (const char* ptr = data; *ptr; ptr++)
		{
			int32_t idx = (-y - hh) * GridSize.h + (x - hw);
			TileInfo& tile = Tiles[idx];
			tile.Position.x = x;
			tile.Position.y = y;
			switch (*ptr)
			{
			case '.':
				tile.Type = 0;
				break;
			case '#':
				tile.Type = 1;
				break;
			default:
				tile.Type = -1;
				break;
			}
			if ((++x) > -hw)
			{
				x = hw;
				if ((--y) < hh)
				{
					break;
				}
			}
		}
	}
	IsChanged = true;
}

ScenePrimitive* TileMap::GetPrimitives()
{
	if (IsChanged)
	{
		uint32_t count = 0;
		for (uint32_t y = 0; y < GridSize.h; y++)
		{
			uint32_t offset = y * GridSize.w;
			for (uint32_t x = 0; x < GridSize.w; x++)
			{
				TileInfo& tile = Tiles[offset + x];
				if (tile.Type >= 0)
				{
					TilePrimitive& prim = Primitives[count];
					prim.Next = nullptr;
					prim.MaterialPtr = MapMaterial;
					prim.CommitParameters = &CommitParameters;
					prim.Info = &tile;
					if (count > 0)
					{
						Primitives[count - 1].Next = &prim;
					}
					++count;
				}
			}
		}
		IsChanged = false;
	}
	return Primitives;
}

void TileMap::Finalize()
{
	TileMapMem.Destroy();
}

void TileMap::CommitParameters(ScenePrimitive* base, void* ptr, size_t max)
{
	TileParameters* params = static_cast<TileParameters*>(ptr);
	TileInfo& tile = *reinterpret_cast<TilePrimitive*>(base)->Info;
	Vec3f offset = { tile.Position.x * 2.f, tile.Position.y * 2.f, 0.f };
	Mat4f_Translate(Mat4f_Identity(&params->ModelTransform), &offset);
	params->FaceNormalAndColor = { 0.f, 0.f, -1.f, TILE_COLOR[tile.Type] };
}
