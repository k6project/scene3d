#include "renderer.hpp"

#include <windows.h>
#include <dxgi.h>
#include <d3d11.h>

#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d11.lib")

/*struct D3D11Mesh
{
	ID3D11Buffer* Buffer;
	unsigned int VBOffset;
	unsigned int VBStride;
	DXGI_FORMAT IBFormat = DXGI_FORMAT_R32_UINT;
	unsigned int IBOffset;
	unsigned int DrawCount;
	unsigned int DrawOffset = 0;
};*/

struct D3D11Primitive
{
	void DrawPrimitive(ID3D11DeviceContext* context) { DrawPrimitiveImpl(this, context); }
	void(*DrawPrimitiveImpl)(void*, ID3D11DeviceContext*) = nullptr;
	D3D11Primitive* Next = nullptr;
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

// Full-screen overlay primitive
struct D3D11OverlayPrimitive : public TD3D11Primitive<D3D11OverlayPrimitive>
{
	void Draw(ID3D11DeviceContext* context)
	{
		context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
		
		//context->Draw(4, 0);
	}
};

class D3D11Renderer : public IRenderer
{
public:
	virtual void Initialize(void* window) override;
	virtual void RenderScene(const Scene* scene) override;
	virtual OverlayRef CreateOverlay() override;
	virtual void Finalize() override;
private:
	void AddPrimitive(D3D11Primitive* primitive);
	IDXGISwapChain* SwapChain;
	ID3D11Device* Device;
	ID3D11DeviceContext* Context;
	ID3D11RenderTargetView* RenderTarget;
	D3D_FEATURE_LEVEL FeatureLevel = D3D_FEATURE_LEVEL_9_1;
	float ClearColor[4] = { 0.f, 0.4f, 0.9f, 1.f };
	D3D11Primitive* FirstPrimitive;
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
		nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0, nullptr, 0, D3D11_SDK_VERSION, 
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
}

void D3D11Renderer::RenderScene(const Scene* scene)
{
	Context->ClearRenderTargetView(RenderTarget, ClearColor);
	for (D3D11Primitive* primitive = FirstPrimitive; primitive != nullptr; primitive = primitive->Next)
	{
		primitive->DrawPrimitive(Context);
	}
	SwapChain->Present(0, 0);
}

IRenderer::OverlayRef D3D11Renderer::CreateOverlay()
{
	D3D11OverlayPrimitive* retval = new D3D11OverlayPrimitive();
	AddPrimitive(retval);
	return retval;
}

void D3D11Renderer::Finalize()
{
	RenderTarget->Release();
	SwapChain->Release();
	Device->Release();
	Context->Release();
}

void D3D11Renderer::AddPrimitive(D3D11Primitive* primitive)
{
	if (FirstPrimitive)
	{
		FirstPrimitive->Next = primitive;
	}
	else
	{
		FirstPrimitive = primitive;
	}
}

IRenderer* IRenderer::Get()
{
	static D3D11Renderer impl = {};
	return &impl;
}
