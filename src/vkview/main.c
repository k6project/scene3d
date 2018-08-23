#include "coredefs.h"
#include "platform.h"
#include "assets.h"

int CALLBACK WinMain(HINSTANCE inst, HINSTANCE prev, LPSTR cmd, int show)
{
	EResult res = AppStartup();
	if (res != RES_NO_ERROR)
	{
		return show;
	}
	//VkInitialize()
	////VkCreatePipeline(shader, fixed)
	////VkCreateVertexArray()
	////VkCreateTexture()
	while (AppIsFinished() != RES_APP_SHUTDOWN)
	{
		//VkRenderScene();
		AppPollEvents();
	}
	AppShutdown();
    return show;
}
