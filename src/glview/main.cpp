#include "utils.hpp"

#include <napp/window.h>
#include <napp/appmain.h>
#include <napp/callback.h>

class glview
{
    void init();
	void destroy();
	void render();
    renderer gfx;
    renderer_t& rndr;
	id_t current_mesh;
public:
    glview() : rndr(get_renderer()) {}
	void run();
};

void glview::init()
{
    rndr.load_material("glview");
    gfx.init();
    gfx.load_material("glview", true);
	gfx.load_meshes({ 
		"tetr", // Tetrahedron
		"cube", // Cube
		"octa", // Octahedron
		"dode", // Dodecahedron
		"icos"  // Icosahedron
	});
    current_mesh = gfx.get_mesh("icos");
}

void glview::destroy()
{
    gfx.destroy();
}

void glview::render()
{
    gfx.begin_frame();
    gfx.draw_mesh(current_mesh);
    gfx.end_frame();
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
