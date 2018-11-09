#include "global.h"

#include "args.h"
#include "vk_api.h"

#include <stdio.h>
#include <shlwapi.h>
#include <windows.h>

static struct
{
	HWND window;
	bool keepRunning;
	AppCallbacks* callbacks;
	void* appState;
} gState =
{
	NULL,
	false,
	NULL,
	NULL
};

#define INVOKE(c) \
    do if((gState.callbacks->##c##)) \
    gState.callbacks->##c##(gState.appState); \
    while(0)

static LRESULT WINAPI appWndProc(HWND wnd, UINT msg, WPARAM w, LPARAM l)
{
	LRESULT retval = 0;
	switch (msg)
	{
	case WM_CREATE:
		gState.keepRunning = true;
		gState.window = wnd;
		INVOKE(beforeStart);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		gState.keepRunning = false;
		break;
	case WM_CLOSE:
		INVOKE(beforeStop);
		DestroyWindow(wnd);
		break;
	default:
		retval = DefWindowProc(wnd, msg, w, l);
		break;
	}
	return retval;
}

bool appShouldKeepRunning(void)
{
	MSG msg;
	while (PeekMessage(&msg, gState.window, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return gState.keepRunning;
}

void appInitialize(HMemAlloc mem, const Options* opts, AppCallbacks* callbacks, void* state)
{
	gState.appState = state;
	gState.callbacks = callbacks;
	WNDCLASS wndClass;
	ZeroMemory(&wndClass, sizeof(wndClass));
	wndClass.style = CS_OWNDC;
	wndClass.hbrBackground = (HBRUSH)COLOR_WINDOW;
	wndClass.lpfnWndProc = (WNDPROC)&appWndProc;
	wndClass.hInstance = GetModuleHandle(NULL);
	wndClass.hIcon = LoadIcon(NULL, IDI_WINLOGO);
	wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndClass.lpszClassName = "APP_WND";
	RegisterClass(&wndClass);
	HMONITOR monitor = MonitorFromWindow(NULL, MONITOR_DEFAULTTOPRIMARY);
	MONITORINFO monitorInfo;
	monitorInfo.cbSize = sizeof(MONITORINFO);
	GetMonitorInfo(monitor, &monitorInfo);
	RECT windowRect = monitorInfo.rcMonitor;
	DWORD wStyle = WS_VISIBLE;
	if (!opts->isFullscreen)
	{
		SetRect(&windowRect, 0, 0, opts->windowWidth, opts->windowHeight);
		wStyle |= WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU;
		AdjustWindowRect(&windowRect, wStyle, FALSE);
	}
	else
		wStyle |= WS_POPUP;
	int rows = windowRect.bottom - windowRect.top;
	int cols = windowRect.right - windowRect.left;
	int left = ((monitorInfo.rcMonitor.right - monitorInfo.rcMonitor.left) - cols) >> 1;
	int top = ((monitorInfo.rcMonitor.bottom - monitorInfo.rcMonitor.top) - rows) >> 1;
	CreateWindow("APP_WND", opts->windowTitle, wStyle, left, top, cols, rows, NULL, NULL, GetModuleHandle(NULL), NULL);
}

bool appLoadLibrary(const char* name, void** handle)
{
	HMODULE ptr = LoadLibrary(name);
	*handle = (ptr != INVALID_HANDLE_VALUE) ? (void*)ptr : NULL;
	return true;
}

void* appGetLibraryProc(void* handle, const char* name)
{
	if (handle)
#ifdef __cplusplus
		return GetProcAddress(static_cast<HMODULE>(handle), name);
#else
		return GetProcAddress(handle, name);
#endif
	return NULL;
}

void appUnloadLibrary(void* handle)
{
	if (handle)
		FreeLibrary((HMODULE)handle);
}

void appPrintf(const char* fmt, ...)
{
	va_list args;
	char buff[256];
	va_start(args, fmt);
	vsprintf_s(buff, 255, fmt, args);
	OutputDebugString(buff);
	va_end(args);
}

bool appCreateVkSurface(VkInstance inst, const VkAllocationCallbacks* alloc, VkSurfaceKHR* surface)
{
	VkWin32SurfaceCreateInfoKHR createInfo;
	memset(&createInfo, 0, sizeof(createInfo));
	createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	createInfo.hinstance = GetModuleHandle(NULL);
	createInfo.hwnd = gState.window;
	VK_ASSERT(vkCreateWin32SurfaceKHR(inst, &createInfo, alloc, surface), "ERROR: %s", "Failed to create surface");
	return true;
}

void appGetName(char* buff, size_t max)
{
	const char* cmdLine = GetCommandLine();
	const char* fileName = PathFindFileName(cmdLine);
	const char* delimPos = StrChr(fileName, '.');
	if (fileName && delimPos && delimPos > fileName)
	{
		size_t len = delimPos - fileName;
		if (len > max)
			len = max;
		memcpy(buff, fileName, len);
	}
}

extern int appMain(int argc, const char** argv);

int CALLBACK WinMain(HINSTANCE inst, HINSTANCE prev, LPSTR cmd, int show)
{
	
	int argc = 0, len = 0, idx = 0;
	for (char* pos = cmd; ; pos++, len++)
	{
		if (isspace(*pos))
		{
			*pos = 0;
			if (len > 0)
				argc++;
			len = -1;
		}
		else if (*pos == 0)
		{
			if (len > 0)
				argc++;
			break;
		}
	}
	const char** argv = _alloca(argc * sizeof(const char*));
	for (char *pos = cmd, *arg = cmd, prev = *cmd; idx < argc; pos++)
	{
		if (*pos == 0 && prev)
		{
			argv[idx++] = arg;
			arg = pos + 1;
		}
		prev = *pos;
	}
	appMain(argc, argv);
	return show;
}
