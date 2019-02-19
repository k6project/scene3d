#include <dxgi.h>
#include <d3d11_1.h>
#include <windows.h>

#include <common.hpp>
#include <memory.hpp>
#include <renderer.hpp>

#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d11.lib")

#define PBUFFER_ALIGN 16
#define RELEASE(o) if((o)){o->Release();o=nullptr;}

struct D3D11Buffer : public TLinkedListNode<D3D11Buffer>
{
    size_t Size;
	ID3D11Buffer* Buffer = nullptr;
	~D3D11Buffer();
};

D3D11Buffer::~D3D11Buffer()
{
	RELEASE(Buffer);
}

struct D3D11Texture : public RendererAPI::Texture, public TLinkedListNode<D3D11Texture>
{
	ID3D11Texture2D* Texture = nullptr;
	~D3D11Texture();
};

D3D11Texture::~D3D11Texture()
{
	RELEASE(Texture);
}

struct D3D11Material : public RendererAPI::Material, public TLinkedListNode<D3D11Material>
{
    ID3D11VertexShader* VertexShader = nullptr;
    ID3D11PixelShader* PixelShader = nullptr;
    ID3D11RasterizerState* RasterizerState = nullptr;
	ID3D11BlendState* BlendState = nullptr;
	ID3D11DepthStencilState* DepthState = nullptr;
	~D3D11Material();
};

D3D11Material::~D3D11Material()
{
	RELEASE(VertexShader);
	RELEASE(PixelShader);
	RELEASE(RasterizerState);
	RELEASE(BlendState);
	RELEASE(DepthState);
}

struct D3D11Primitive : public TLinkedListNode<D3D11Primitive>
{
	void DrawPrimitive(ID3D11DeviceContext* context) { DrawPrimitiveImpl(this, context); }
	void(*DrawPrimitiveImpl)(void*, ID3D11DeviceContext*) = nullptr;
};

template <typename T>
void D3D11DrawPrimitive(void* ptr, ID3D11DeviceContext* context)
{
	T* primitive = static_cast<T*>(ptr);
	(primitive->*(&T::Draw))(context);
}

template <typename T>
struct TD3D11Primitive : public D3D11Primitive
{
	TD3D11Primitive() { DrawPrimitiveImpl = &D3D11DrawPrimitive<T>; }
};

class D3D11Renderer : public RendererAPI
{
public:
	virtual bool HasRHClipSpace() const override { return false; }
	virtual float GetAspectRatio() const override { return (Viewport.Width / Viewport.Height); }
	virtual void Initialize(void* window, size_t pbSize, size_t gpSize) override;
	virtual void RenderScene(const Scene* scene) override;
	virtual void Finalize() override;
	virtual void CreateTexture(const TextureDescriptor& desc, Texture** texturePtr) override;
	virtual void CreateMaterial(const MaterialDescriptor& info, Material** materialPtr) override;
protected:
	void CreateBuffer(const D3D11_BUFFER_DESC& desc, D3D11Buffer*& buffer);
	uint32_t ParamOffset(size_t bytes) const { return (bytes >> 4) & UINT32_MAX; }
	D3D11Buffer* Parameters = nullptr;
private:
	static const D3D_FEATURE_LEVEL REQUIRED_FEATURE_LEVEL = D3D_FEATURE_LEVEL_11_1;
	MemAllocLinear LocalMemory;
	IDXGISwapChain* SwapChain;
	ID3D11Device1* Device;
	ID3D11DeviceContext1* Context;
	ID3D11RenderTargetView* RenderTarget;
	D3D_FEATURE_LEVEL FeatureLevel = D3D_FEATURE_LEVEL_9_1;
	float ClearColor[4] = { 0.f, 0.f, 0.f, 1.f };//{ 0.f, 0.4f, 0.9f, 1.f };
	D3D11Texture* Textures = nullptr;
    D3D11Material* Materials = nullptr;
	D3D11Primitive* Primitives = nullptr;
	D3D11Buffer* Buffers = nullptr;
	ID3D11InputLayout* VBOLayout;
	D3D11_VIEWPORT Viewport;
	HWND Window;
};

void D3D11Renderer::Initialize(void* window, size_t pbSize, size_t gpSize)
{
	RECT windowRect;
	Window = static_cast<HWND>(window);
	GetWindowRect(Window, &windowRect);
	DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
	swapChainDesc.BufferDesc.Width = windowRect.right - windowRect.left;
	swapChainDesc.BufferDesc.Height = windowRect.bottom - windowRect.top;
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
	swapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
	swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = 1;
	swapChainDesc.OutputWindow = Window;
	swapChainDesc.Windowed = TRUE;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	ID3D11Device* TmpDevice = nullptr;
	ID3D11DeviceContext* TmpContext = nullptr;
	HRESULT hr = D3D11CreateDeviceAndSwapChain(
		nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 
		D3D11_CREATE_DEVICE_DEBUG, &REQUIRED_FEATURE_LEVEL, 1, D3D11_SDK_VERSION,
		&swapChainDesc, &SwapChain, &TmpDevice, &FeatureLevel, &TmpContext
	);
	if (FAILED(hr))
	{
		MessageBox(Window, "Failed to initialize D3D11", "Fatal", MB_OK);
		return;
	}
	else if (FeatureLevel < REQUIRED_FEATURE_LEVEL)
	{
		MessageBox(Window, "Graphics context uses feature level below Direct3D 11.1", "Fatal", MB_OK);
		return;
	}
	else
	{
		void *devPtr = nullptr, *ctxPtr = nullptr;
		if (FAILED(TmpDevice->QueryInterface(__uuidof(ID3D11Device1), &devPtr)))
		{
			MessageBox(Window, "Failed to retrieve Direct3D 11.1 interface", "Fatal", MB_OK);
			return;
		}
		if (FAILED(TmpContext->QueryInterface(__uuidof(ID3D11DeviceContext1), &ctxPtr)))
		{
			MessageBox(Window, "Failed to retrieve Direct3D 11.1 interface", "Fatal", MB_OK);
			return;
		}
		Context = reinterpret_cast<ID3D11DeviceContext1*>(ctxPtr);
		Device = reinterpret_cast<ID3D11Device1*>(devPtr);
		TmpContext->Release();
		TmpDevice->Release();
	}
	ID3D11Texture2D* SwapChainImage = nullptr;
	hr = SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&SwapChainImage));
	if (FAILED(hr))
	{
		MessageBox(Window, "Failed to get back buffer image", "Fatal", MB_OK);
		return;
	}
	hr = Device->CreateRenderTargetView(SwapChainImage, NULL, &RenderTarget);
	SwapChainImage->Release();
	if (FAILED(hr))
	{
		MessageBox(Window, "Failed to create render target", "Fatal", MB_OK);
		return;
	}
	Context->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	Context->OMSetRenderTargets(1, &RenderTarget, nullptr);
	Viewport.TopLeftX = static_cast<float>(windowRect.left);
	Viewport.TopLeftY = static_cast<float>(windowRect.top);
    Viewport.Width = static_cast<float>(windowRect.right - windowRect.left);
    Viewport.Height = static_cast<float>(windowRect.bottom - windowRect.top);
    Context->RSSetViewports(1, &Viewport);
	D3D11_BUFFER_DESC desc = {};
	desc.ByteWidth = pbSize & UINT_MAX;
	desc.Usage = D3D11_USAGE_DYNAMIC;
	desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	D3D11Buffer* pBuff = nullptr;
	CreateBuffer(desc, pBuff);
	uint32_t pOffset = 0, pSize = ParamOffset(ALIGN(gpSize, 256));
	Context->VSSetConstantBuffers1(0, 1, &pBuff->Buffer, &pOffset, &pSize);
	Context->PSSetConstantBuffers1(0, 1, &pBuff->Buffer, &pOffset, &pSize);
	Parameters = pBuff;
}

void D3D11Renderer::RenderScene(const Scene* scene)
{
	D3D11_MAPPED_SUBRESOURCE mappedBuff;
	ID3D11Buffer* pBuffer = Parameters->Buffer;
	Context->Map(Parameters->Buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedBuff);
	scene->CommitParameters(mappedBuff.pData, Parameters->Size);
	Context->Unmap(Parameters->Buffer, 0);
	Context->ClearRenderTargetView(RenderTarget, ClearColor);
	for (const ScenePrimitive* ptr = scene->GetPrimitives(); ptr != nullptr; ptr = ptr->Next)
	{
		const ScenePrimitive& prim = *ptr;
		const D3D11Material* material = static_cast<D3D11Material*>(prim.MaterialPtr);
		ID3D11Buffer* pBuffers[] = { pBuffer/*, pBuffer*/ };
		uint32_t pBufferCount = ArrayLength(pBuffers) & UINT32_MAX;
        uint32_t pbOffset[] = 
		{ 
			ParamOffset(prim.LocalParameters.Offset)
		};
        uint32_t pbLength[] = 
		{ 
			ParamOffset(prim.LocalParameters.Length) 
		};
		Context->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        Context->VSSetConstantBuffers1(1, pBufferCount, pBuffers, pbOffset, pbLength);
        Context->PSSetConstantBuffers1(1, pBufferCount, pBuffers, pbOffset, pbLength);
        Context->VSSetShader(material->VertexShader, nullptr, 0);
        Context->PSSetShader(material->PixelShader, nullptr, 0);
        Context->RSSetState(material->RasterizerState);
		Context->Draw(6, 0);
	}

    /*for (D3D11Material* m = Materials; m != nullptr; m = m->Next)
    {
        Context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
        Context->VSSetShader(m->VertexShader, nullptr, 0);
        Context->PSSetShader(m->PixelShader, nullptr, 0);
        Context->RSSetState(m->RasterizerState);
        Context->Draw(4, 0);
    }*/
	SwapChain->Present(0, 0);
}

void D3D11Renderer::Finalize()
{
	DeleteAll(Textures);
	DeleteAll(Materials);
	DeleteAll(Buffers);
	RenderTarget->Release();
	SwapChain->Release();
	Device->Release();
	Context->Release();
}

void D3D11Renderer::CreateBuffer(const D3D11_BUFFER_DESC& desc, D3D11Buffer*& buffer)
{
	D3D11Buffer* buff = new D3D11Buffer();
    Device->CreateBuffer(&desc, nullptr, &buff->Buffer);
    buff->Size = desc.ByteWidth;
    buff->Next = Buffers;
    Buffers = buff;
    buffer = buff;
}

void D3D11Renderer::CreateTexture(const TextureDescriptor& desc, Texture** texturePtr)
{
	D3D11Texture* texture = new D3D11Texture();
	D3D11_TEXTURE2D_DESC tDesc = {};
	tDesc.Width = desc.Width;
	tDesc.Height = desc.Height;
	switch (desc.Format)
	{
	case RendererAPI::FormatBGRA8Unorm:
		tDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
		break;
	case RendererAPI::FormatD24UnormS8Uint:
		tDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		break;
	}
	tDesc.ArraySize = 1;
	tDesc.MipLevels = 1;
	tDesc.SampleDesc.Count = 1;
	tDesc.SampleDesc.Quality = 0;
	tDesc.Usage = D3D11_USAGE_IMMUTABLE;
	tDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	D3D11_SUBRESOURCE_DATA data = {};
	/*data.pSysMem = desc.Data;
	data.SysMemPitch = desc.Width;
	Device->CreateTexture2D(&tDesc, &data, &texture->Texture);*/
	texture->Next = Textures;
	Textures = texture;
	*texturePtr = texture;
}

void D3D11Renderer::CreateMaterial(const MaterialDescriptor& desc, Material** materialPtr)
{
	D3D11Material* material = new D3D11Material();
	Device->CreateVertexShader(desc.VertexShader, desc.VertexShader.Size(), nullptr, &material->VertexShader);
	Device->CreatePixelShader(desc.PixelShader, desc.PixelShader.Size(), nullptr, &material->PixelShader);
	D3D11_RASTERIZER_DESC rsDesc = {};
	rsDesc.FillMode = D3D11_FILL_SOLID;
	switch (desc.Culling)
	{
	case RendererAPI::NoCulling:
		rsDesc.CullMode = D3D11_CULL_NONE;
		break;
	case RendererAPI::BackFaceCW:
		rsDesc.CullMode = D3D11_CULL_BACK;
		rsDesc.FrontCounterClockwise = FALSE;
		break;
	case RendererAPI::BackFaceCCW:
	default:
		rsDesc.CullMode = D3D11_CULL_BACK;
		rsDesc.FrontCounterClockwise = TRUE;
		break;
	}
	Device->CreateRasterizerState(&rsDesc, &material->RasterizerState);
	material->Next = Materials;
	Materials = material;
	*materialPtr = material;
}

RendererAPI* RendererAPI::Get()
{
	static D3D11Renderer impl = {};
	return &impl;
}
