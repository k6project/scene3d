#include <napp/napp_main.h>

static void NAppMain()
{
	if (NAppInitialize())
	{
		NAppSetFullscreen(false);
		NAppSetViewSize(1280, 800);
		//NAppSetStartupProc()
		//NAppSetShutdownProc()
		//NAppSetUpdateProc()
		NAppRun();
	}
}
