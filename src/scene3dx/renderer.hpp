#pragma once

class Scene;

struct IBuffer {};

struct IBufferInfo
{
	virtual void InitForParameterBuffer(size_t bytes) = 0;
};

struct IMaterialInfo
{
    virtual void SetVertexShader(const char* name) = 0;
    virtual void SetPixelShader(const char* name) = 0;
    virtual void SetBackfaceCulling(bool value) = 0;
};

class RendererAPI
{
public:
	enum PBUpdateType
	{
		PBUpdateFull,
		PBUpdatePartial,
	};

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

	struct ParameterBlock
	{};

	static RendererAPI* Get();
	virtual void Initialize(void* window) = 0;
	virtual void RenderScene(const Scene* scene) = 0;
	virtual void Finalize() = 0;
	virtual void AllocateParameterBlock(size_t size, ParameterBlock** block) = 0;
	virtual void UpdateParameterBlock(const ParameterBlock* block, const void* data, size_t size) = 0;
	virtual void BeginParameterBufferUpdate(PBUpdateType type) = 0;
	virtual void CommitParameterBufferUpdate() = 0;
    virtual IMaterialInfo* NewMaterialInfo() = 0;
    virtual Material* CreateMaterial(const IMaterialInfo* info) = 0;
};

typedef RendererAPI::ParameterBlock* ParameterBlock;
typedef RendererAPI::MaterialDesc* MaterialDesc;
typedef RendererAPI::Material* Material;
