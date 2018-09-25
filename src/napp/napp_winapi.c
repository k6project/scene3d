#include "napp.h"

#ifdef _NAPP_WINAPI_

#include <windows.h>

static void* NAppStartupDummy(void)
{
	return NULL;
}

static void NAppLifecycleDummy(void* arg)
{
}

static struct  
{
	HWND window;
	RECT view_rect;
	DWORD wnd_style;
	MONITORINFO monitor_info;
	bool is_initialized;
	bool is_fullscreen;
	bool is_running;
	void* (*startup_proc)(void);
	void (*shutdown_proc)(void*);
	void (*update_proc)(void*);
	void* global_state;
} g_app;

static LRESULT WINAPI WndProc(HWND wnd, UINT msg, WPARAM w, LPARAM l)
{
	LRESULT retval = 0;
	switch (msg)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		g_app.is_running = false;
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
		g_app.wnd_style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_VISIBLE;
	}
	g_app.is_fullscreen = value;
}

void NAppSetViewSize(int width, int height)
{
	SetRect(&g_app.view_rect, 0, 0, width, height);
}


bool NAppInitialize()
{
	if (!g_app.is_initialized)
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
			g_app.monitor_info.cbSize = sizeof(MONITORINFO);
			HMONITOR Monitor = MonitorFromWindow(NULL, MONITOR_DEFAULTTOPRIMARY);
			GetMonitorInfo(Monitor, &g_app.monitor_info);
			g_app.is_initialized = true;
		}
		else
		{
			//AppMessage("Failed to register window class");
		}
		g_app.startup_proc = &NAppStartupDummy;
		g_app.shutdown_proc = &NAppLifecycleDummy;
		g_app.update_proc = &NAppLifecycleDummy;
	}
	return g_app.is_initialized;
}

void NAppRun()
{
	LPSTR title = "";
	RECT windowRect = g_app.view_rect;
	HINSTANCE inst = GetModuleHandle(NULL);
	AdjustWindowRect(&g_app.view_rect, g_app.wnd_style, FALSE);
	int rows = windowRect.bottom - windowRect.top;
	int cols = windowRect.right - windowRect.left;
	int left = ((g_app.monitor_info.rcMonitor.right - g_app.monitor_info.rcMonitor.left) - cols) >> 1;
	int top = ((g_app.monitor_info.rcMonitor.bottom - g_app.monitor_info.rcMonitor.top) - rows) >> 1;
	g_app.window = CreateWindow("NAPP_WND", title, g_app.wnd_style, left, top, cols, rows, NULL, NULL, inst, NULL);
	if (g_app.window)
	{
		MSG msg;
		g_app.is_running = true;
		g_app.global_state = g_app.startup_proc();
		while (g_app.is_running)
		{
			while (PeekMessage(&msg, g_app.window, 0, 0, PM_REMOVE))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			g_app.update_proc(g_app.global_state);
		}
		g_app.shutdown_proc(g_app.global_state);
	}
}

void napp_set_startup_proc(void*(*proc)(void))
{
    g_app.startup_proc = (proc) ? proc : &NAppStartupDummy;
}

void napp_set_main_loop_proc(void(*proc)(void*))
{
    g_app.update_proc = (proc) ? proc : &NAppLifecycleDummy;
}

void napp_set_shutdown_proc(void(*proc)(void*))
{
    g_app.shutdown_proc = (proc) ? proc : &NAppLifecycleDummy;
}

#endif
