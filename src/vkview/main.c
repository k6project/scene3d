/*
#include <napp/appmain.h>

in C:

void app_main_loop(void* arg)
{
	app_context_t* ctx = (app_context_t*)arg;
	...
}

static void napp_main()
{
	if (napp_initialize())
	{
		app_context_t my_app;
		napp_set_context(&my_app);
		napp_set_callback(NAPP_MAIN_LOOP, &app_main_loop);
		napp_run();
	}
}

in C++:

class app_context_t
{
public:
	void main_loop(void*)
	{
		...
	}
	void run()
	{
		napp_set_callback(NAPP_MAIN_LOOP, [](void* arg){main_loop(arg);});
		napp_run();
	}
}

static void napp_main()
{
	if (napp_initialize())
	{
		app_context_t my_app;
		my_app.run();
	}
}

*/

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
