#include <dxgi.h>
#include <d3d11.h>
#include <windows.h>

#include <common.hpp>
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

struct D3D11ParameterBuffer : public D3D11Buffer, public RendererAPI::ParameterBuffer
{
    ID3D11DeviceContext* Context = nullptr;
    virtual void Update(const void* data, size_t size) override;
};

void D3D11ParameterBuffer::Update(const void* data, size_t size)
{
    D3D11_MAPPED_SUBRESOURCE sRes = {};
    if (SUCCEEDED(Context->Map(Buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &sRes)))
    {
        memcpy_s(sRes.pData, Size, data, size);
        Context->Unmap(Buffer, 0);
    }
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

struct D3D11MaterialDesc : public RendererAPI::MaterialDesc
{
    virtual void SetShader(RendererAPI::ShaderStage stage, const char* name) override;
    virtual void SetBackfaceCulling(bool value) override;
    TScopedPtr<char> VSCode, PSCode;
    size_t VSSize = 0, PSSize = 0;
    D3D11_RASTERIZER_DESC RSDesc;
};

void D3D11MaterialDesc::SetShader(RendererAPI::ShaderStage stage, const char* name)
{
    switch (stage)
    {
    case RendererAPI::VertexShader:
        VSCode = LoadFileIntoMemory(name, &VSSize);
        break;
    case RendererAPI::PixelShader:
        PSCode = LoadFileIntoMemory(name, &PSSize);
        break;
    default:
        break;
    }
}

void D3D11MaterialDesc::SetBackfaceCulling(bool value)
{
    RSDesc.CullMode = (value) ? D3D11_CULL_BACK : D3D11_CULL_NONE;
}

class D3D11Renderer : public RendererAPI
{
public:
	virtual void Initialize(void* window) override;
	virtual void RenderScene(const Scene* scene) override;
	virtual void Finalize() override;
	virtual void CreateParameterBuffer(size_t size, ParameterBuffer** bufferPtr) override;
    virtual void CreateMaterialDescriptor(MaterialDesc** desc) override;
    virtual void CreateMaterial(const MaterialDesc* info, Material** material) override;
private:
	IDXGISwapChain* SwapChain;
	ID3D11Device* Device;
	ID3D11DeviceContext* Context;
	ID3D11RenderTargetView* RenderTarget;
	D3D_FEATURE_LEVEL FeatureLevel = D3D_FEATURE_LEVEL_9_1;
	float ClearColor[4] = { 0.f, 0.4f, 0.9f, 1.f };
    D3D11Material* Materials = nullptr;
	D3D11Primitive* Primitives = nullptr;
	D3D11Buffer* Buffers = nullptr;
	ID3D11InputLayout* VBOLayout;
	D3D11_VIEWPORT Viewport;
	HWND Window;
};

void D3D11Renderer::Initialize(void* window)
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
	HRESULT hr = D3D11CreateDeviceAndSwapChain(
		nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, D3D11_CREATE_DEVICE_DEBUG, nullptr, 0, D3D11_SDK_VERSION,
		&swapChainDesc, &SwapChain, &Device, &FeatureLevel, &Context
	);
	if (FAILED(hr))
	{
		MessageBox(Window, "Failed to initialize D3D11", "Fatal", MB_OK);
		return;
	}
	else if (FeatureLevel < D3D_FEATURE_LEVEL_11_0)
	{
		MessageBox(Window, "Graphics context uses feature level below Direct3D 11", "Warning", MB_OK);
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
}

void D3D11Renderer::RenderScene(const Scene* scene)
{
	Context->ClearRenderTargetView(RenderTarget, ClearColor);
    for (D3D11Material* m = Materials; m != nullptr; m = m->Next)
    {
        Context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
        Context->VSSetShader(m->VertexShader, nullptr, 0);
        Context->PSSetShader(m->PixelShader, nullptr, 0);
        Context->RSSetState(m->RasterizerState);
        Context->Draw(4, 0);
    }
	SwapChain->Present(0, 0);
}

void D3D11Renderer::Finalize()
{
	DeleteAll(Materials);
	DeleteAll(Buffers);
	RenderTarget->Release();
	SwapChain->Release();
	Device->Release();
	Context->Release();
}

void D3D11Renderer::CreateParameterBuffer(size_t size, RendererAPI::ParameterBuffer** bufferPtr)
{
	*bufferPtr = nullptr;
	if (size > 0)
	{
		size = (((size - 1) / PBUFFER_ALIGN) + 1) * PBUFFER_ALIGN;
        D3D11_BUFFER_DESC desc = {};
        desc.ByteWidth = size & UINT_MAX;
        desc.Usage = D3D11_USAGE_DYNAMIC;
        desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        D3D11ParameterBuffer* buffer = new D3D11ParameterBuffer();
        Device->CreateBuffer(&desc, nullptr, &buffer->Buffer);
        buffer->Context = Context;
        buffer->Size = size;
        buffer->Next = Buffers;
        Buffers = buffer;
        *bufferPtr = buffer;
    }
}

void D3D11Renderer::CreateMaterialDescriptor(MaterialDesc** mDesc)
{
    D3D11MaterialDesc* desc = new D3D11MaterialDesc();
    desc->RSDesc.FillMode = D3D11_FILL_SOLID;
    desc->RSDesc.CullMode = D3D11_CULL_BACK;
    desc->RSDesc.FrontCounterClockwise = TRUE;
    desc->RSDesc.DepthBias = 0;
    desc->RSDesc.SlopeScaledDepthBias = 0.f;
    desc->RSDesc.DepthBiasClamp = 0.f;
    desc->RSDesc.DepthClipEnable = true;
    desc->RSDesc.ScissorEnable = false;
    desc->RSDesc.MultisampleEnable = false;
    desc->RSDesc.AntialiasedLineEnable = false;
    *mDesc = desc;
}

void D3D11Renderer::CreateMaterial(const MaterialDesc* desc, Material** materialPtr)
{
    D3D11Material* material = new D3D11Material();
    const D3D11MaterialDesc* mInfo = static_cast<const D3D11MaterialDesc*>(desc);
    Device->CreateVertexShader(mInfo->VSCode, mInfo->VSSize, nullptr, &material->VertexShader);
    Device->CreatePixelShader(mInfo->PSCode, mInfo->PSSize, nullptr, &material->PixelShader);
    Device->CreateRasterizerState(&mInfo->RSDesc, &material->RasterizerState);
    material->Next = Materials;
    Materials = material;
    delete desc;
    *materialPtr = material;
}

RendererAPI* RendererAPI::Get()
{
	static D3D11Renderer impl = {};
	return &impl;
}
