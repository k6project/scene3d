#include <dxgi.h>
#include <d3d11.h>
#include <windows.h>

#include <common.hpp>
#include <renderer.hpp>

#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d11.lib")

struct D3D11Material : public IMaterial, public LinkedListNode<D3D11Material>
{
    ID3D11VertexShader* VertexShader = nullptr;
    ID3D11PixelShader* PixelShader = nullptr;
    ID3D11RasterizerState* RasterizerState = nullptr;
};

struct D3D11Primitive : public LinkedListNode<D3D11Primitive>
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

struct D3D11MaterialInfo : public IMaterialInfo
{
    virtual void SetVertexShader(const char* name) override;
    virtual void SetPixelShader(const char* name) override;
    virtual void SetBackfaceCulling(bool value) override;
    ScopedPtr<char> VSCode, PSCode;
    size_t VSSize = 0, PSSize = 0;
    D3D11_RASTERIZER_DESC RSDesc;
};

void D3D11MaterialInfo::SetVertexShader(const char* name)
{
    VSCode = LoadFileIntoMemory(name, &VSSize);
}

void D3D11MaterialInfo::SetPixelShader(const char* name)
{
    PSCode = LoadFileIntoMemory(name, &PSSize);
}

void D3D11MaterialInfo::SetBackfaceCulling(bool value)
{
    RSDesc.CullMode = D3D11_CULL_NONE;
}

class D3D11Renderer : public IRenderer
{
public:
	virtual void Initialize(void* window) override;
	virtual void RenderScene(const Scene* scene) override;
    virtual IMaterialInfo* NewMaterialInfo() override;
    virtual IMaterial* CreateMaterial(const IMaterialInfo* info) override;
	virtual void Finalize() override;
private:
	IDXGISwapChain* SwapChain;
	ID3D11Device* Device;
	ID3D11DeviceContext* Context;
	ID3D11RenderTargetView* RenderTarget;
	D3D_FEATURE_LEVEL FeatureLevel = D3D_FEATURE_LEVEL_9_1;
	float ClearColor[4] = { 0.f, 0.4f, 0.9f, 1.f };
    D3D11Material* Materials = nullptr;
	D3D11Primitive* Primitives = nullptr;
	ID3D11InputLayout* VBOLayout;
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
    D3D11_VIEWPORT viewport = {};
    viewport.Width = swapChainDesc.BufferDesc.Width;
    viewport.Height = swapChainDesc.BufferDesc.Height;
    Context->RSSetViewports(1, &viewport);
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
    /*for (D3D11Primitive* primitive = Primitives; primitive != nullptr; primitive = primitive->Next)
	{
		primitive->DrawPrimitive(Context);
	}*/
	SwapChain->Present(0, 0);
}

IMaterialInfo* D3D11Renderer::NewMaterialInfo()
{
    D3D11MaterialInfo* info = new D3D11MaterialInfo();
    info->RSDesc.FillMode = D3D11_FILL_SOLID;
    info->RSDesc.CullMode = D3D11_CULL_BACK;
    info->RSDesc.FrontCounterClockwise = FALSE;
    info->RSDesc.DepthBias = 0;
    info->RSDesc.SlopeScaledDepthBias = 0.f;
    info->RSDesc.DepthBiasClamp = 0.f;
    info->RSDesc.DepthClipEnable = true;
    info->RSDesc.ScissorEnable = false;
    info->RSDesc.MultisampleEnable = false;
    info->RSDesc.AntialiasedLineEnable = false;
    return info;
}

IMaterial* D3D11Renderer::CreateMaterial(const IMaterialInfo* info)
{
    D3D11Material* material = new D3D11Material();
    const D3D11MaterialInfo* mInfo = static_cast<const D3D11MaterialInfo*>(info);
    Device->CreateVertexShader(mInfo->VSCode, mInfo->VSSize, nullptr, &material->VertexShader);
    Device->CreatePixelShader(mInfo->PSCode, mInfo->PSSize, nullptr, &material->PixelShader);
    Device->CreateRasterizerState(&mInfo->RSDesc, &material->RasterizerState);
    material->Next = Materials;
    Materials = material;
    delete info;
    return material;
}

void D3D11Renderer::Finalize()
{
    for (D3D11Material* m = Materials; m != nullptr;)
    {
        D3D11Material* ptr = m;
        ptr->VertexShader->Release();
        ptr->PixelShader->Release();
        ptr->RasterizerState->Release();
        m = ptr->Next;
        delete ptr;
    }
	RenderTarget->Release();
	SwapChain->Release();
	Device->Release();
	Context->Release();
}

IRenderer* IRenderer::Get()
{
	static D3D11Renderer impl = {};
	return &impl;
}
