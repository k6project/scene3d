#include <napp/window.h>
#include <napp/appmain.h>
#include <napp/callback.h>

#include <windows.h>
#include <gl/glcorearb.h>
#include <gl/wgl.h>

#pragma comment(lib, "opengl32.lib")

class glview
{
	HDC dc = nullptr;
	HGLRC glrc = nullptr;
	void init();
	void destroy();
	void render();
public:
	void run();
};

void glview::init()
{
	//glCreateContextNAPP(); - init 4.1 for desktop
	PIXELFORMATDESCRIPTOR pfd;
	memset(&pfd, 0, sizeof(PIXELFORMATDESCRIPTOR));
	pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DOUBLEBUFFER | PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 32;
	pfd.cDepthBits = 32;
	pfd.iLayerType = PFD_MAIN_PLANE;
	dc = GetDC(static_cast<HWND>(napp_get_window_handle()));
	if (int pixel_format = ChoosePixelFormat(dc, &pfd))
	{
		if (SetPixelFormat(dc, pixel_format, &pfd))
		{
			HGLRC tmp = wglCreateContext(dc);
			wglMakeCurrent(dc, tmp);
			PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB = reinterpret_cast<PFNWGLCREATECONTEXTATTRIBSARBPROC>(wglGetProcAddress("wglCreateContextAttribsARB"));
			if (wglCreateContextAttribsARB)
			{
				int attribs[] =
				{
					WGL_CONTEXT_MAJOR_VERSION_ARB, 4, 
					WGL_CONTEXT_MINOR_VERSION_ARB, 1,
					WGL_CONTEXT_FLAGS_ARB, 0, 0
				};
				glrc = wglCreateContextAttribsARB(dc, 0, attribs);
				wglMakeCurrent(nullptr,	nullptr);
				wglDeleteContext(tmp);
				wglMakeCurrent(dc, glrc);
			}
			else
			{
				glrc = tmp;
			}
		}
	}
}

void glview::destroy()
{
	wglMakeCurrent(nullptr, nullptr);
	if (glrc)
	{
		wglDeleteContext(glrc);
	}
}

void glview::render()
{
	SwapBuffers(dc);
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
