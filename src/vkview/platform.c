#include "coredefs.h"
#include "platform.h"

#ifdef _MSC_VER

#include <assert.h>
#include <stdarg.h>
#include <stdio.h>

// NATIVE WINAPI IMPLEMENTATION

static struct  
{
	BOOL IsClosing;
	RECT WindowRect;
	HWND WindowHandle;
} GAppState;

static LRESULT WINAPI WndProc(HWND wnd, UINT msg, WPARAM w, LPARAM l)
{
	LRESULT retval = 0;
	switch (msg)
	{
		case WM_DESTROY: 
			PostQuitMessage(0); 
			GAppState.IsClosing = TRUE; 
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

// APPLICATION INTERFACE

EResult AppStartup()
{
	WNDCLASS wndClass;
	LPCSTR className = WNDCLS_NAME;
	GAppState.IsClosing = FALSE;
	GAppState.WindowHandle = NULL;
	ZeroMemory(&wndClass, sizeof(wndClass));
	wndClass.style = CS_OWNDC;
	wndClass.hbrBackground = (HBRUSH)COLOR_WINDOW;
	wndClass.lpfnWndProc = (WNDPROC)&WndProc;
	wndClass.hInstance = GetModuleHandle(NULL);
	wndClass.hIcon = LoadIcon(NULL, IDI_WINLOGO);
	wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndClass.lpszClassName = className;
	if (!RegisterClass(&wndClass))
	{
		AppMessage("Failed to register window class");
		return RES_PLATFORM_ERROR;
	}
	MONITORINFO monitor_info = { .cbSize = sizeof(MONITORINFO) };
	HMONITOR Monitor = MonitorFromWindow(NULL, MONITOR_DEFAULTTONEAREST);
	GetMonitorInfo(Monitor, &monitor_info);
	SetRect(&GAppState.WindowRect, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
	DWORD style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_VISIBLE;
	AdjustWindowRect(&GAppState.WindowRect, style, FALSE);
	int rows = GAppState.WindowRect.bottom - GAppState.WindowRect.top;
	int cols = GAppState.WindowRect.right - GAppState.WindowRect.left;
	int left = ((monitor_info.rcMonitor.right - monitor_info.rcMonitor.left) - cols) >> 1;
	int top = ((monitor_info.rcMonitor.bottom - monitor_info.rcMonitor.top) - rows) >> 1;
	LPSTR title = PROGRAM_NAME;
	HINSTANCE inst = GetModuleHandle(NULL);
	GAppState.WindowHandle = CreateWindow(className, title, style, left, top, cols, rows, NULL, NULL, inst, NULL);
	if (!GAppState.WindowHandle)
	{
		AppMessage("CreateWindow returned NULL");
		return RES_PLATFORM_ERROR;
	}
	return RES_NO_ERROR;
}

EResult AppIsFinished()
{
	return (GAppState.IsClosing) ? RES_APP_SHUTDOWN : RES_NO_ERROR;
}

void AppPollEvents()
{
	MSG msg;
	while (PeekMessage(&msg, GAppState.WindowHandle, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}

void AppShutdown()
{
}

void AppMessage(const char* format, ...)
{
	va_list args;
	char msg[1024];
	va_start(args, format);
	vsnprintf_s(msg, sizeof(msg), _TRUNCATE, format, args);
	va_end(args);
	MessageBox(GAppState.WindowHandle, msg, "", MB_OK);
}

#endif // _MSC_VER
