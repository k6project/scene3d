#include <windows.h>

#include <common.hpp>
#include <renderer.hpp>
#include <scene3dx.hpp>

bool Scene3DXApp::ShouldKeepRunning() const
{
	return KeepRunning;
}

void Scene3DXApp::Initialize(void* window)
{
	Renderer = RendererAPI::Get();
	Renderer->Initialize(window);
	MaterialDescriptor mInfo;
    mInfo.SetShader(RendererAPI::VertexShader, "OverlayVertexShader.cso");
    mInfo.SetShader(RendererAPI::PixelShader, "OverlayPixelShader.cso");
    Material* material;
    Renderer->CreateMaterial(mInfo, &material);
	//IPrimitiveInfo* pInfo = Renderer->NewPrimitiveInfo();
	//pInfo->SetVertexBufferData(void*, size_t);
	//pInfo->SetTopology(TRIANGLE_STRIP);
	//pInfo->SetMaterial(material);
	//IPrimitive* primitive = Renderer->CreatePrimitive(pInfo);

	//Uniforms:
	//Renderer->AllocateParameterBlock(size, params); // allocate a block within parameter buffer
	
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
	tDesc.Data = new uint32_t[size * size];
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
	}
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
	void *pBuffer = nullptr;
	//Renderer->InitParameterBuffer(); //map
	//Renderer->UpdateParameterBlock(GlobalsBlock, &GlobalParameters, sizeof(GlobalParameters));
	//Renderer->CommitParameterBuffer(); //unmap/flush
	Renderer->RenderScene(nullptr);
}

void Scene3DXApp::Finalize()
{
	KeepRunning = false;
	Renderer->Finalize();
}

