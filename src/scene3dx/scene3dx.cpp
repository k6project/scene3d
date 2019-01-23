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
	static const uint32_t defColorRGBA[] = 
	{	
		0xFF0000FFu, 0x00FF00FFu, 0x0000FFFFu, 0xFFFF00FFu
	};
	static const int size = 64;
	static const int count = sizeof(defColorRGBA) / sizeof(defColorRGBA[0]);
	uint32_t* bytes = new uint32_t[size * size * count];
	for (int i = 0; i < count; i++)
	{
		//TextureDescriptor tDesc;
		uint32_t* ptr = bytes + (i * size * size);
		for (int y = 0; y < size; y++)
		{
			int scanline = y * size;
			for (int x = 0; x < size; x++)
			{

			}
		}
		// init tDesc;
		// create texture;
	}
	delete[] bytes;
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

