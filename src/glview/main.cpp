#include <windows.h>

#include <napp/window.h>
#include <napp/appmain.h>
#include <napp/callback.h>

class glview
{
	void init();
	void destroy();
public:
	void run();
};

void glview::init()
{
}

void glview::destroy()
{
}

void glview::run()
{
	napp_set_context(this);
	napp_set_fullscreen(false);
	napp_set_window_size(512, 512);
	napp_set_callback(NAPP_STARTUP, &napp_cb<glview, &glview::init>);
	napp_set_callback(NAPP_SHUTDOWN, &napp_cb<glview, &glview::destroy>);
	napp_run();
}

static void napp_main()
{
	if (napp_initialize())
	{
		glview().run();
	}
}
