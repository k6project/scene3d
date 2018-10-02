#include <cstdio>
#include <cstdlib>

#include <gfx/opengl.h>
#include <napp/window.h>
#include <napp/appmain.h>
#include <napp/callback.h>
#include <napp/filesys.h>

void asset_load_bmd(const char* fname)
{
    buffer_t rdbuff;
    buffer_init(&rdbuff, nullptr, 0);
    napp_fs_load_file(fname, rdbuff);
    const struct bmd_t
    {
        unsigned int v_count;
        unsigned int i_count;
        struct
        {
            unsigned int idx : 8;
            int size         : 8;
            int stride       : 8;
            unsigned int offs: 8;
        } attribs[3];
        unsigned int v_bytes;
        unsigned int i_offset;
        unsigned int i_bytes;
    }& bmd_head = buffer_data_ref<bmd_t>(rdbuff);
    buffer_free(rdbuff);
}

class glview
{
	GLuint shader = 0;
    void init();
	void destroy();
	void render();
public:
	void run();
};

void glview::init()
{
    glCreateContextNAPP();
	GLenum stage_type[] = { GL_VERTEX_SHADER, GL_FRAGMENT_SHADER };
	const char* stage_fname[] = { "glview.vert", "glview.frag", nullptr };
	shader = glCreateShaderProgramNAPP(stage_fname, stage_type);
	glClearColor(0.4f, 0.6f, 1.0f, 1.f);
	glClear(GL_COLOR_BUFFER_BIT);
	glUseProgram(shader);
    asset_load_bmd("tetr.bmd");
}

void glview::destroy()
{
	glUseProgram(0);
	glDeleteProgram(shader);
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
