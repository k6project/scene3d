#pragma once

#include <common.hpp>

struct Scene;

class RendererAPI
{
public:
	enum FaceCulling
	{
		NoCulling,
		BackFaceCW,
		BackFaceCCW
	};

	enum DataFormat
	{
		FormatBGRA8Unorm,
		FormatD24UnormS8Uint
	};

	struct TextureDescriptor
	{
		uint32_t Width = 0, Height = 0;
		DataFormat Format;
		//TScopedPtr<uint32_t> Data;
	};

	struct MaterialDescriptor
	{
		IOBuffer VertexShader;
		IOBuffer PixelShader;
		FaceCulling Culling = BackFaceCCW;
	};

	struct Texture
	{};

	struct Material 
	{};

	static RendererAPI* Get();
	virtual bool HasRHClipSpace() const = 0;
	virtual float GetAspectRatio() const = 0;
	virtual void Initialize(void* window, size_t pbSize, size_t gpSize) = 0;
	virtual void CreateTexture(const TextureDescriptor& desc, Texture** texturePtr) = 0;
	virtual void CreateMaterial(const MaterialDescriptor& desc, Material** materialPtr) = 0;
	virtual void RenderScene(const Scene* scene) = 0;
	virtual void Finalize() = 0;
};

typedef RendererAPI::TextureDescriptor TextureDescriptor;
typedef RendererAPI::MaterialDescriptor MaterialDescriptor;
typedef RendererAPI::Texture Texture;
typedef RendererAPI::Material Material;

struct ScenePrimitive
{
	ScenePrimitive* Next;
    uint32_t ParamOffset[3];
    uint32_t ParamLength[3];
	Material* MaterialPtr;
};
