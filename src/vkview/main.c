#include <napp/napp_main.h>

/*int CALLBACK WinMain(HINSTANCE inst, HINSTANCE prev, LPSTR cmd, int show)
{
	EResult res = AppStartup();
	if (res != RES_NO_ERROR)
	{
		return show;
	}
	while (AppIsFinished() != RES_APP_SHUTDOWN)
	{
		AppPollEvents();
	}
	AppShutdown();
    return show;
}*/

static void NAppMain()
{
	if (NAppInitialize())
	{
		NAppSetFullscreen(false);
		NAppSetViewSize(1280, 800);
		NAppRun();
	}
}
