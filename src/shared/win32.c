#include "main.h"
#include "args.inl"
#include "vk_api.h"

#include <shlwapi.h>
#include <stdarg.h>

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

void appInitialize(AppCallbacks* callbacks, void* state)
{
	gState.appState = state;
	gState.callbacks = callbacks;

	const TChar* cmdLine = GetCommandLine();
	const TChar* fileName = PathFindFileNameW(cmdLine);
	const TChar* delimPos = StrChr(fileName, '.');
	if (fileName && delimPos && delimPos > fileName)
	{ 
		ptrdiff_t len = delimPos - fileName;
		if (len > APP_NAME_LEN)
			len = APP_NAME_LEN;
		memcpy(gOptions_.appName, fileName, len * sizeof(TChar));
		gOptions_.appName[APP_NAME_LEN] = 0;
	}
	WNDCLASS wndClass;
	ZeroMemory(&wndClass, sizeof(wndClass));
	wndClass.style = CS_OWNDC;
	wndClass.hbrBackground = (HBRUSH)COLOR_WINDOW;
	wndClass.lpfnWndProc = (WNDPROC)&appWndProc;
	wndClass.hInstance = GetModuleHandle(NULL);
	wndClass.hIcon = LoadIcon(NULL, IDI_WINLOGO);
	wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndClass.lpszClassName = L"APP_WND";
	RegisterClass(&wndClass);
	HMONITOR monitor = MonitorFromWindow(NULL, MONITOR_DEFAULTTOPRIMARY);
	MONITORINFO monitorInfo;
	monitorInfo.cbSize = sizeof(MONITORINFO);
	GetMonitorInfo(monitor, &monitorInfo);
	RECT windowRect = monitorInfo.rcMonitor;
	DWORD wStyle = WS_VISIBLE;
	if (!gOptions->isFullscreen)
	{
		SetRect(&windowRect, 0, 0, gOptions_.windowWidth, gOptions->windowHeight);
		wStyle |= WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU;
		AdjustWindowRect(&windowRect, wStyle, FALSE);
	}
	else
		wStyle |= WS_POPUP;
	int rows = windowRect.bottom - windowRect.top;
	int cols = windowRect.right - windowRect.left;
	int left = ((monitorInfo.rcMonitor.right - monitorInfo.rcMonitor.left) - cols) >> 1;
	int top = ((monitorInfo.rcMonitor.bottom - monitorInfo.rcMonitor.top) - rows) >> 1;
	CreateWindow(L"APP_WND", gOptions_.windowTitle, wStyle, left, top, cols, rows, NULL, NULL, GetModuleHandle(NULL), NULL);
}

void appPollEvents(void)
{
}

bool appLoadLibrary(const TChar* name, void** handle)
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

void appTCharToUTF8(char* dest, const TChar* src, int max)
{
	WideCharToMultiByte(CP_UTF8, WC_NO_BEST_FIT_CHARS, src, -1, dest, max, NULL, NULL);
}

void appPrintf(const TChar* fmt, ...)
{
	va_list args;
	TChar buff[256];
	va_start(args, fmt);
	vswprintf_s(buff, 255, fmt, args);
	OutputDebugString(buff);
	va_end(args);
}

#ifndef NO_VULKAN
bool vkCreateSurfaceAPP(VkInstance inst, const VkAllocationCallbacks* alloc, VkSurfaceKHR* surface)
{
	VkWin32SurfaceCreateInfoKHR createInfo;
	memset(&createInfo, 0, sizeof(createInfo));
	createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	createInfo.hinstance = GetModuleHandle(NULL);
	createInfo.hwnd = gState.window;
	VK_ASSERT(vkCreateWin32SurfaceKHR(inst, &createInfo, alloc, surface), "ERROR: Failed to create surface");
	return true;
}
#endif
