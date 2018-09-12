#include "napp.h"

#include <windows.h>

static struct  
{
	HWND Window;
	RECT ViewRect;
	MONITORINFO MonitorInfo;
	bool IsInitialized;
} NApp;

static LRESULT WINAPI WndProc(HWND wnd, UINT msg, WPARAM w, LPARAM l)
{
	LRESULT retval = 0;
	switch (msg)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		//GAppState.IsClosing = TRUE;
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
			HMONITOR Monitor = MonitorFromWindow(NULL, MONITOR_DEFAULTTONEAREST);
			GetMonitorInfo(Monitor, &NApp.MonitorInfo);
			NApp.IsInitialized = true;
		}
		else
		{
			//AppMessage("Failed to register window class");
		}
	}
	return NApp.IsInitialized;
}

void NAppRun()
{
	//todo create window
	MSG msg;
	while (PeekMessage(&msg, NApp.Window, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}
