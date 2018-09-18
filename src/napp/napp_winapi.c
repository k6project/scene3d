#include "napp.h"

#ifdef _NAPP_WINAPI_

#include <windows.h>

static void* NAppStartupDummy()
{
	return NULL;
}

static void NAppLifecycleDummy(void* arg)
{
}

static struct  
{
	HWND Window;
	RECT ViewRect;
	DWORD WindowStyle;
	MONITORINFO MonitorInfo;
	bool IsInitialized;
	bool IsFullscreen;
	bool IsRunning;
	NAppStartupProc StartupProc;
	NAppShutdownProc ShutdownProc;
	NAppUpdateProc UpdateProc;
	void* GlobalState;
} NApp;

static LRESULT WINAPI WndProc(HWND wnd, UINT msg, WPARAM w, LPARAM l)
{
	LRESULT retval = 0;
	switch (msg)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		NApp.IsRunning = false;
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

NAPP_API void NAppArgv(LPSTR cmd)
{
}

void NAppSetFullscreen(bool value)
{
	if (value)
	{
	}
	else
	{
		NApp.WindowStyle = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_VISIBLE;
	}
	NApp.IsFullscreen = value;
}

void NAppSetViewSize(int width, int height)
{
	SetRect(&NApp.ViewRect, 0, 0, width, height);
}


bool NAppInitialize()
{
	if (!NApp.IsInitialized)
	{
		WNDCLASS wndClass;
		ZeroMemory(&wndClass, sizeof(wndClass));
		wndClass.style = CS_OWNDC;
		wndClass.hbrBackground = (HBRUSH)COLOR_WINDOW;
		wndClass.lpfnWndProc = (WNDPROC)&WndProc;
		wndClass.hInstance = GetModuleHandle(NULL);
		wndClass.hIcon = LoadIcon(NULL, IDI_WINLOGO);
		wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
		wndClass.lpszClassName = "NAPP_WND";
		if (RegisterClass(&wndClass))
		{
			NApp.MonitorInfo.cbSize = sizeof(MONITORINFO);
			HMONITOR Monitor = MonitorFromWindow(NULL, MONITOR_DEFAULTTOPRIMARY);
			GetMonitorInfo(Monitor, &NApp.MonitorInfo);
			NApp.IsInitialized = true;
		}
		else
		{
			//AppMessage("Failed to register window class");
		}
		NApp.StartupProc = &NAppStartupDummy;
		NApp.ShutdownProc = &NAppLifecycleDummy;
		NApp.UpdateProc = &NAppLifecycleDummy;
	}
	return NApp.IsInitialized;
}

void NAppRun()
{
	LPSTR title = "";
	RECT windowRect = NApp.ViewRect;
	HINSTANCE inst = GetModuleHandle(NULL);
	AdjustWindowRect(&NApp.ViewRect, NApp.WindowStyle, FALSE);
	int rows = windowRect.bottom - windowRect.top;
	int cols = windowRect.right - windowRect.left;
	int left = ((NApp.MonitorInfo.rcMonitor.right - NApp.MonitorInfo.rcMonitor.left) - cols) >> 1;
	int top = ((NApp.MonitorInfo.rcMonitor.bottom - NApp.MonitorInfo.rcMonitor.top) - rows) >> 1;
	NApp.Window = CreateWindow("NAPP_WND", title, NApp.WindowStyle, left, top, cols, rows, NULL, NULL, inst, NULL);
	if (NApp.Window)
	{
		MSG msg;
		NApp.IsRunning = true;
		NApp.GlobalState = NApp.StartupProc();
		while (NApp.IsRunning)
		{
			while (PeekMessage(&msg, NApp.Window, 0, 0, PM_REMOVE))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			NApp.UpdateProc(NApp.GlobalState);
		}
		NApp.ShutdownProc(NApp.GlobalState);
	}
}

#endif
