#pragma once

#include <napp/macros.h>

#if defined(_NAPP_WINAPI_)

#pragma comment(lib, "opengl32.lib")
#include <windows.h>
#include <gl/glcorearb.h>
#define GLFUNCTION(a,b) NAPP_API PFNGL##b##PROC gl##a##; 
#include "glfunc.inl"

#elif defined(_NAPP_MACOS_)

#include <OpenGL/gl.h>
#define glGenVertexArrays glGenVertexArraysAPPLE
#define glBindVertexArray glBindVertexArrayAPPLE
#define glDeleteVertexArrays glDeleteVertexArraysAPPLE

#endif

NAPP_API void glCreateContextNAPP();

NAPP_API void glDestroyContextNAPP();

NAPP_API void glSwapBuffersNAPP();

NAPP_API GLuint glCreateShaderProgramNAPP(const char** files, GLenum* stages);
