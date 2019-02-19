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
	// Renderer_Get(); // not necessary for singleton
	virtual bool HasRHClipSpace() const = 0;
	// Renderer_HasRHClipSpace();
	virtual float GetAspectRatio() const = 0;
	// Renderer_GetAspectRatio();
	virtual void Initialize(void* window, size_t pbSize, size_t gpSize) = 0;
	// Renderer_Initialize(void*, size_t, size_t);
	virtual void CreateTexture(const TextureDescriptor& desc, Texture** texturePtr) = 0;
	// Renderer_CreateTexture(const TextureDescriptor*, Texutre**);
	virtual void CreateMaterial(const MaterialDescriptor& desc, Material** materialPtr) = 0; 
	// Renderer_CreateMaterial(const MaterialDescriptor*, Material**);
	virtual void RenderScene(const Scene* scene) = 0;	
	// Renderer_RenderScene(const Scene*);
	virtual void Finalize() = 0;
};

typedef RendererAPI::TextureDescriptor TextureDescriptor;
typedef RendererAPI::MaterialDescriptor MaterialDescriptor;
typedef RendererAPI::Texture Texture;
typedef RendererAPI::Material Material;

struct ScenePrimitive
{
	ScenePrimitive* Next;
	void(*CommitParameters)(ScenePrimitive*, void*, size_t);
	struct { size_t Offset, Length; } LocalParameters;
	Material* MaterialPtr;
};
