#include <windows.h>

#include <common.hpp>
#include <renderer.hpp>

class Scene3DXApp
{
public:
	bool ShouldKeepRunning() const;
	void Initialize(HWND window);
	void Update(float deltaT);
	void Finalize();
private:
	bool KeepRunning = false;
	IRenderer* Renderer;
};

bool Scene3DXApp::ShouldKeepRunning() const
{
	return KeepRunning;
}

void Scene3DXApp::Initialize(HWND window)
{
	Renderer = IRenderer::Get();
	Renderer->Initialize(window);
    IMaterialInfo* mInfo = Renderer->NewMaterialInfo();
    mInfo->SetVertexShader("OverlayVertexShader.cso");
    mInfo->SetPixelShader("OverlayPixelShader.cso");
    IMaterial* material = Renderer->CreateMaterial(mInfo);
	//IPrimitiveInfo* pInfo = Renderer->NewPrimitiveInfo();
	//pInfo->SetVertexBufferData(void*, size_t);
	//pInfo->SetTopology(TRIANGLE_STRIP);
	//pInfo->SetMaterial(material);
	//IPrimitive* primitive = Renderer->CreatePrimitive(pInfo);
	KeepRunning = true;
}

void Scene3DXApp::Update(float deltaT)
{
	Renderer->RenderScene(nullptr);
}

void Scene3DXApp::Finalize()
{
	KeepRunning = false;
	Renderer->Finalize();
}

static LRESULT WINAPI WndProc(HWND wnd, UINT msg, WPARAM w, LPARAM l)
{
	static Scene3DXApp* state = nullptr;
	LRESULT retval = 0;
	switch (msg)
	{
	case WM_CREATE:
		state = reinterpret_cast<Scene3DXApp*>(reinterpret_cast<LPCREATESTRUCT>(l)->lpCreateParams);
		state->Initialize(wnd);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		state->Finalize();
		break;
	case WM_CLOSE:
		DestroyWindow(wnd);
		break;
	default:
		retval = DefWindowProc(wnd, msg, w, l);
		break;
	}
	return retval;
}

int CALLBACK WinMain(HINSTANCE inst, HINSTANCE prev, LPSTR cmd, int show)
{
	static const char* clsName = "SCENE3DX_APP_WND";
	Scene3DXApp state = {};
	WNDCLASS wndClass = {};
	wndClass.style = CS_OWNDC;
	wndClass.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW);
	wndClass.lpfnWndProc = reinterpret_cast<WNDPROC>(&WndProc);
	wndClass.hInstance = GetModuleHandle(NULL);
	wndClass.hIcon = LoadIcon(NULL, IDI_WINLOGO);
	wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndClass.lpszClassName = clsName;
	RegisterClass(&wndClass);
	HMONITOR monitor = MonitorFromWindow(NULL, MONITOR_DEFAULTTOPRIMARY);
	MONITORINFO monitorInfo;
	monitorInfo.cbSize = sizeof(MONITORINFO);
	GetMonitorInfo(monitor, &monitorInfo);
	RECT windowRect = monitorInfo.rcMonitor;
	DWORD wStyle = WS_VISIBLE | WS_POPUP;
	int rows = windowRect.bottom - windowRect.top;
	int cols = windowRect.right - windowRect.left;
	int left = ((monitorInfo.rcMonitor.right - monitorInfo.rcMonitor.left) - cols) >> 1;
	int top = ((monitorInfo.rcMonitor.bottom - monitorInfo.rcMonitor.top) - rows) >> 1;
	HWND hwnd = CreateWindow(clsName, nullptr, wStyle, left, top, cols, rows, NULL, NULL, GetModuleHandle(NULL), &state);
	while (state.ShouldKeepRunning())
	{
		MSG msg;
		state.Update(0.f);
		while (PeekMessage(&msg, hwnd, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
    return show;
}
