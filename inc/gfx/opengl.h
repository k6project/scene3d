#pragma once

#include <napp/macros.h>

#ifdef _NAPP_WINAPI_
#pragma comment(lib, "opengl32.lib")
#include <windows.h>
#include <gl/glcorearb.h>
#define GLFUNCTION(a,b) NAPP_API PFNGL##b##PROC gl##a##; 
#include "glfunc.inl"
#endif

NAPP_API void glCreateContextNAPP();

NAPP_API void glDestroyContextNAPP();

NAPP_API void glSwapBuffersNAPP();
