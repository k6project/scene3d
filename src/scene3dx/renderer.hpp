#pragma once

#ifndef RENDERER_IMPL
#define RENDERER_IMPL private
#endif

class Scene;

class RendererAPI
{
public:
	enum ShaderStage
	{
		VertexShader,
		PixelShader,
		NumShaderStages
	};

	enum FaceCulling
	{
		NoCulling,
		BackFaceCW,
		BackFaceCCW
	};

	class MaterialDescriptor
	{
	public:
		void SetShader(ShaderStage stage, const char* name);
		void SetCulling(FaceCulling value);
	RENDERER_IMPL:
		TScopedPtr<char> VSCode, PSCode;
		size_t VSSize = 0, PSSize = 0;
		FaceCulling Culling = BackFaceCCW;
	};

	struct Material 
	{};

	struct ParameterBuffer
	{
        virtual void Update(const void* data, size_t size) = 0;
    };

	static RendererAPI* Get();
	virtual void Initialize(void* window) = 0;
	virtual void RenderScene(const Scene* scene) = 0;
	virtual void Finalize() = 0;
	virtual void CreateParameterBuffer(size_t size, ParameterBuffer** bufferPtr) = 0;
    virtual void CreateMaterial(const MaterialDescriptor& info, Material** materialPtr) = 0;
};

typedef RendererAPI::MaterialDescriptor MaterialDescriptor;
typedef RendererAPI::ParameterBuffer* ParameterBuffer;
typedef RendererAPI::Material* Material;
