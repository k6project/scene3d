#include <gfx/opengl.h>
#include <napp/window.h>
#include <napp/appmain.h>
#include <napp/callback.h>

class glview
{
    void init();
	void destroy();
	void render();
public:
	void run();
};

void glview::init()
{
    glCreateContextNAPP();
	glClearColor(0.4f, 0.6f, 1.0f, 1.f);
	glClear(GL_COLOR_BUFFER_BIT);
}

void glview::destroy()
{
    glDestroyContextNAPP();
}

void glview::render()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	glSwapBuffersNAPP();
}

void glview::run()
{
	napp_set_context(this);
	napp_set_fullscreen(false);
	napp_set_window_size(512, 512);
	napp_set_callback(NAPP_UPDATE_END, &napp_cb<glview, &glview::render>);
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
