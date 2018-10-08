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
	HGLRC glrc;
	HDC dc;
} g_app;

static LRESULT WINAPI napp_wnd_proc(HWND wnd, UINT msg, WPARAM w, LPARAM l)
{
	LRESULT retval = 0;
	switch (msg)
	{
	case WM_CREATE:
		g_app.window = wnd;
		napp_invoke_cb(NAPP_STARTUP);
		break;
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
	napp_init_callbacks();
	HINSTANCE inst = GetModuleHandle(NULL);
	AdjustWindowRect(&windowRect, g_app.wnd_style, FALSE);
	int rows = windowRect.bottom - windowRect.top;
	int cols = windowRect.right - windowRect.left;
	int left = ((g_app.monitor_info.rcMonitor.right - g_app.monitor_info.rcMonitor.left) - cols) >> 1;
	int top = ((g_app.monitor_info.rcMonitor.bottom - g_app.monitor_info.rcMonitor.top) - rows) >> 1;
	CreateWindow("NAPP_WND", title, g_app.wnd_style, left, top, cols, rows, NULL, NULL, inst, NULL);
	if (g_app.window)
	{
		MSG msg;
		g_app.is_running = true;
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

void* napp_get_window_handle()
{
	return g_app.window;
}

// IF USING OPENGL

#include <gl/glcorearb.h>
#define GLFUNCTION(a,b) PFNGL##b##PROC gl##a## = NULL; 
#include <gfx/glfunc.inl>

#include <gl/wgl.h>

void glCreateContextNAPP()
{
	PIXELFORMATDESCRIPTOR pfd;
	memset(&pfd, 0, sizeof(PIXELFORMATDESCRIPTOR));
	pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DOUBLEBUFFER | PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 32;
	pfd.cDepthBits = 24;
	pfd.cStencilBits = 8;
	pfd.iLayerType = PFD_MAIN_PLANE;
	g_app.dc = GetDC(g_app.window);
	HMODULE dll = GetModuleHandle("opengl32.dll");
	int pf = ChoosePixelFormat(g_app.dc, &pfd);
	if (dll && pf)
	{
		if (SetPixelFormat(g_app.dc, pf, &pfd))
		{
			HGLRC tmp = wglCreateContext(g_app.dc);
			wglMakeCurrent(g_app.dc, tmp);
			PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB = (PFNWGLCREATECONTEXTATTRIBSARBPROC)(wglGetProcAddress("wglCreateContextAttribsARB"));
			if (wglCreateContextAttribsARB)
			{
				int attribs[] =
				{
					WGL_CONTEXT_MAJOR_VERSION_ARB, 4,
					WGL_CONTEXT_MINOR_VERSION_ARB, 1,
					WGL_CONTEXT_FLAGS_ARB, 0, 0
				};
				g_app.glrc = wglCreateContextAttribsARB(g_app.dc, 0, attribs);
				wglMakeCurrent(NULL, NULL);
				wglDeleteContext(tmp);
				wglMakeCurrent(g_app.dc, g_app.glrc);
#define GLFUNCTION(a,b) gl##a## = (PFNGL##b##PROC)(wglGetProcAddress("gl"#a));
#define GLFUNCTION_OLD(a,b) gl##a## = (PFNGL##b##PROC)(GetProcAddress(dll, "gl"#a));
#include <gfx/glfunc.inl>
			}
			else
			{
				g_app.glrc = tmp;
			}
		}
	}
}

void glDestroyContextNAPP()
{
	wglMakeCurrent(NULL, NULL);
	if (g_app.glrc)
	{
	    wglDeleteContext(g_app.glrc);
	}
}

void glSwapBuffersNAPP()
{
	SwapBuffers(g_app.dc);
}

#endif
