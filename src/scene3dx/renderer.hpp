#pragma once

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

	struct MaterialDesc
	{
		virtual void SetShader(ShaderStage stage, const char* name) = 0;
		virtual void SetBackfaceCulling(bool value) = 0;
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
    virtual void CreateMaterialDescriptor(MaterialDesc** desc) = 0;
    virtual void CreateMaterial(const MaterialDesc* info, Material** materialPtr) = 0;
};

typedef RendererAPI::ParameterBuffer* ParameterBuffer;
typedef RendererAPI::MaterialDesc* MaterialDesc;
typedef RendererAPI::Material* Material;
