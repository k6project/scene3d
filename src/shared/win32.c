#include "main.h"
#include "args.inl"

static struct
{
	HWND window;
	bool keepRunning;
	AppCallbacks* callbacks;
	void* appState;
} gState =
{
	INVALID_HANDLE_VALUE,
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
		gState.window = wnd;
		gState.keepRunning = true;
		INVOKE(beforeStart);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		gState.keepRunning = false;
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

bool appShouldKeepRunning(void)
{
	return gState.keepRunning;
}

void appInitialize(AppCallbacks* callbacks, void* state)
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
	wndClass.lpszClassName = L"APP_WND";
	RegisterClass(&wndClass);
	/*HMONITOR monitor = MonitorFromWindow(NULL, MONITOR_DEFAULTTOPRIMARY);
	MONITORINFO monitorInfo;
	GetMonitorInfo(monitor, &monitorInfo);*/
	RECT windowRect;
	SetRect(&windowRect, 0, 0, gOptions->windowWidth, gOptions->windowHeight);
	DWORD wStyle = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_VISIBLE;
	AdjustWindowRect(&windowRect, wStyle, FALSE);
	int rows = windowRect.bottom - windowRect.top;
	int cols = windowRect.right - windowRect.left;
	int left = 0;// ((monitorInfo.rcMonitor.right - monitorInfo.rcMonitor.left) - cols) >> 1;
	int top = 0;// ((monitorInfo.rcMonitor.bottom - monitorInfo.rcMonitor.top) - rows) >> 1;
	CreateWindow(L"APP_WND", L"", wStyle, left, top, cols, rows, NULL, NULL, GetModuleHandle(NULL), NULL);
}

void appPollEvents(void)
{
	MSG msg;
	while (PeekMessage(&msg, gState.window, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}
