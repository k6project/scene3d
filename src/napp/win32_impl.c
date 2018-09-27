#ifdef _NAPP_WINAPI_

#include <windows.h>

#include "common.h"

#include <napp/window.h>
#include <napp/appmain.h>

static struct win32_app_t
{
	HWND window;
	RECT view_rect;
	DWORD wnd_style;
	MONITORINFO monitor_info;
	bool is_initialized;
	bool is_fullscreen;
	bool is_running;
} g_app;

static LRESULT WINAPI napp_wnd_proc(HWND wnd, UINT msg, WPARAM w, LPARAM l)
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

NAPP_API void napp_argv(LPSTR cmd)
{
}

void napp_set_fullscreen(bool value)
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

void napp_set_window_size(int width, int height)
{
	SetRect(&g_app.view_rect, 0, 0, width, height);
}


bool napp_initialize()
{
	if (!g_app.is_initialized)
	{
		WNDCLASS wndClass;
		ZeroMemory(&wndClass, sizeof(wndClass));
		wndClass.style = CS_OWNDC;
		wndClass.hbrBackground = (HBRUSH)COLOR_WINDOW;
		wndClass.lpfnWndProc = (WNDPROC)&napp_wnd_proc;
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
	}
	return g_app.is_initialized;
}

void napp_run()
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
		napp_init_callbacks();
		g_app.is_running = true;
		napp_invoke_cb(NAPP_STARTUP);
		while (g_app.is_running)
		{
			napp_invoke_cb(NAPP_UPDATE_BEGIN);
			while (PeekMessage(&msg, g_app.window, 0, 0, PM_REMOVE))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			napp_invoke_cb(NAPP_UPDATE_END);
		}
		napp_invoke_cb(NAPP_SHUTDOWN);
	}
}

#endif
