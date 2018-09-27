/*
#include <napp/appmain.h>
#include <napp/callback.h>

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
		napp_set_callback(NAPP_UPDATE_END, &app_main_loop);
		napp_run();
	}
}

in C++:

 class app_t
 {
 public:
     void tick()
     {
     }
     void run()
     {
        napp_set_context(this);
        napp_set_callback(NAPP_UPDATE_END, &napp_cb<app_t, &app_t::tick>);
        napp_run();
     }
 };
 
 static void napp_main()
 {
     if (napp_initialize())
     {
        app_t my_app;
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
