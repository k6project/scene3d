#include <windows.h>

#include <common.hpp>
#include <renderer.hpp>
#include <scene3dx.hpp>

#define MAX_PRIMITIVES 256

struct GlobalParameters
{
    Matrix4f Projection;
    Matrix4f ViewTransform;
};

struct LocalParameters
{
    Matrix4f ModelTransform;
};

void Scene3DXApp::CommitParameters(void* buffer, size_t max) const
{
    Parameters.Reset();
    GlobalParameters* globals = Parameters.TAlloc<GlobalParameters>(1, 256);
    //build globals->Projection from camera settings
    //build globals->ViewTransform from camera settings
    //create linear allocator, allocating with 16 byte granularity
    //allocate globals, allocate object-specifics
    Parameters.CopyTo(buffer, max);
}

const ScenePrimitive* Scene3DXApp::GetPrimitives() const
{
	return nullptr;
}

bool Scene3DXApp::ShouldKeepRunning() const
{
	return KeepRunning;
}

void Scene3DXApp::Initialize(void* window)
{
	Renderer = RendererAPI::Get();
    size_t globals = ALIGN(sizeof(GlobalParameters), 256);
    size_t perPrimitive = MAX_PRIMITIVES * ALIGN(sizeof(LocalParameters), 256);
    Parameters.Init(globals + perPrimitive);
	Renderer->Initialize(window, Parameters.GetCapacity(), globals);

	MaterialDescriptor mInfo;
	mInfo.VertexShader.LoadFromFile("OverlayVertexShader.cso");
	mInfo.PixelShader.LoadFromFile("OverlayPixelShader.cso");
    Material* material = nullptr;
    Renderer->CreateMaterial(mInfo, &material);
	CreateTextures();
	CreateMaterials();

	KeepRunning = true;
}

void Scene3DXApp::CreateTextures()
{
	static const uint32_t defColorBGRA[] = 
	{	
		0x0000FFFFu, 0x00FF00FFu, 0xFF0000FFu, // RGB
		0xFFFF00FFu, 0xFF00FFFFu, 0x00FFFFFFu, // CMY
		0x000000FFu, 0xFFFFFFFFu			   // BW
	};
	static const uint32_t size = 64;
	static const int count = sizeof(defColorBGRA) / sizeof(defColorBGRA[0]);
	TextureDescriptor tDesc = {};
	tDesc.Width = size;
	tDesc.Height = size;
	tDesc.Format = RendererAPI::FormatBGRA8Unorm;
	/*tDesc.Data = new uint32_t[size * size];
	for (int i = 0; i < count; i++)
	{
		for (uint32_t y = 0; y < size; y++)
		{
			uint32_t scanline = y * size;
			for (uint32_t x = 0; x < size; x++)
			{
				tDesc.Data[scanline + x] = defColorBGRA[i];
			}
		}
		Texture* texture;
		Renderer->CreateTexture(tDesc, &texture);
	}*/
}

void Scene3DXApp::CreateMaterials()
{
	// 2D Sprite material
	{
		//MaterialDescriptor desc;
		//desc.SetShader(RendererAPI::VertexShader, "SpriteVertexShader");
		//desc.SetShader(RendererAPI::PixelShader, "SpritePixelShader");
		//Renderer->CreateMaterial(desc, &SpriteMaterial);
	}
}

void Scene3DXApp::Update(float deltaT)
{
	Renderer->RenderScene(this);
}

void Scene3DXApp::Finalize()
{
	KeepRunning = false;
	Renderer->Finalize();
}

