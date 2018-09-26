#include <napp/napp_main.h>

static void napp_main()
{
	if (napp_initialize())
	{
		napp_set_fullscreen(false);
		napp_set_view_size(1280, 800);
		napp_run();
	}
}
