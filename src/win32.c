#include "global.h"

#include "args.h"

#include <stdio.h>
#include <shlwapi.h>
#include <windows.h>

#define VK_NO_PROTOTYPES
#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>

static struct
{
	HWND window;
	bool keepRunning;
	void* appState;
} gState =
{
	NULL,
	false,
	NULL
};

extern void appOnStartup(void* dataPtr);

extern void appOnShutdown(void* dataPtr);

static LRESULT WINAPI appWndProc(HWND wnd, UINT msg, WPARAM w, LPARAM l)
{
	LRESULT retval = 0;
	switch (msg)
	{
	case WM_CREATE:
		gState.keepRunning = true;
		gState.window = wnd;
		appOnStartup(gState.appState);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		gState.keepRunning = false;
		break;
	case WM_CLOSE:
		appOnShutdown(gState.appState);
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

void appInitialize(MemAlloc mem, const Options* opts, void* state)
{
	gState.appState = state;
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

bool sysLoadLibrary(const char* name, void** handle)
{
	HMODULE ptr = LoadLibrary(name);
	*handle = (ptr != INVALID_HANDLE_VALUE) ? (void*)ptr : NULL;
	return true;
}

void* sysGetLibraryProc(void* handle, const char* name)
{
	if (handle)
#ifdef __cplusplus
		return GetProcAddress(static_cast<HMODULE>(handle), name);
#else
		return GetProcAddress(handle, name);
#endif
	return NULL;
}

void sysUnloadLibrary(void* handle)
{
	if (handle)
		FreeLibrary((HMODULE)handle);
}

void sysPrintf(const char* fmt, ...)
{
#ifdef _DEBUG
	va_list args;
	va_start(args, fmt);
	vfprintf(stdout, fmt, args);
	va_end(args);
#endif
}

const void* sysGetVkSurfaceInfo()
{
	static VkWin32SurfaceCreateInfoKHR createInfo;
	createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	createInfo.hinstance = GetModuleHandle(NULL);
	createInfo.hwnd = gState.window;
	return &createInfo;
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

void* sysLoadFile(const char* path, size_t* size, MemAlloc mem, MemAllocMode mode)
{
	char root[MAX_PATH + 1];
#ifdef _DEBUG
	DWORD length = GetCurrentDirectory(MAX_PATH + 1, root);
	char* delim = &root[length];
#else
	DWORD length = GetModuleFileName(NULL, root, MAX_PATH + 1);
	char* delim = StrRChr(root, NULL, '\\');
#endif
	for (const char *ch = path;; ch++, delim++)
	{
		*delim = (*ch == '/') ? '\\' : *ch;
		if (!*ch) break;
	}
	HANDLE fp = CreateFile(root, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	ASSERT_Q(fp != INVALID_HANDLE_VALUE);
	void* retVal = NULL;
	LARGE_INTEGER fSize;
	TEST_Q(GetFileSizeEx(fp, &fSize));
	size_t bytes = fSize.LowPart;
	switch (mode)
	{
		case MEM_FORWD: retVal = memForwdAlloc(mem, bytes);
		case MEM_STACK: retVal = memStackAlloc(mem, bytes);
		case MEM_HEAP: retVal = memHeapAlloc(mem, bytes);
	}
	TEST_Q(ReadFile(fp, retVal, bytes & UINT32_MAX, NULL, NULL));
	CloseHandle(fp);
	*size = bytes;
	return retVal;
}

extern int appMain(int argc, const char** argv);

int CALLBACK WinMain(HINSTANCE inst, HINSTANCE prev, LPSTR cmd, int show)
{
#if _DEBUG
	AllocConsole();
	AttachConsole(GetCurrentProcessId());
	freopen("CON", "w", stdout);
#endif
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
