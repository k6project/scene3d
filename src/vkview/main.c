#include "coredefs.h"
#include "platform.h"
#include "assets.h"

int CALLBACK WinMain(HINSTANCE inst, HINSTANCE prev, LPSTR cmd, int show)
{
	Image image, *tmp = &image;
	TGALoadImage(TEXTURE_FILE(4pixel), &tmp);
	return 0;

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
