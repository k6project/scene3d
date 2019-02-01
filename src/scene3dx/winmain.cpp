#include <windows.h>

#include <common.hpp>
#include <scene3dx.hpp>

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
	Scene3DXApp app;
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
	HWND hwnd = CreateWindow(clsName, nullptr, wStyle, left, top, cols, rows, NULL, NULL, GetModuleHandle(NULL), &app);
	while (app.ShouldKeepRunning())
	{
		MSG msg;
		app.Update(0.f);
		while (PeekMessage(&msg, hwnd, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	return show;
}
