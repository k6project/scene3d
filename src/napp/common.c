#include "common.h"

#include <stdio.h>
#include <gfx/opengl.h>

static void* g_context;

static napp_callback_t g_callback[NAPP_CALLBACKS];

static void napp_dummy(void* arg)
{
}

void napp_init_callbacks()
{
	for (int i = 0; i < NAPP_CALLBACKS; i++)
	{
		if (!g_callback[i])
			g_callback[i] = &napp_dummy;
	}
}

void napp_set_context(void* context)
{
	g_context = context;
}

void napp_set_callback(unsigned int index, napp_callback_t proc)
{
	if (index < NAPP_CALLBACKS)
	{
		g_callback[index] = proc;
	}
}

void napp_invoke_cb(unsigned int index)
{
	g_callback[index](g_context);
}

GLuint glCreateShaderProgramNAPP(const char** files, GLenum* stages)
{
	GLuint result = 0;
	void* rdbuff = NULL;
	int buff_max = 0, count;
	result = glCreateProgram();
	for (int i = 0; files[i]; i++)
	{
		GLenum stage = stages[i];
		const char* fname = files[i];
		FILE* fp = fopen(fname, "rb");
		if (fp)
		{
			fseek(fp, 0, SEEK_END);
			count = ftell(fp) & INT_MAX;
			fseek(fp, 0, SEEK_SET);
			if (!rdbuff || count > buff_max)
			{
				buff_max = (((count - 1) / 1024) + 1) * 1024;
				rdbuff = realloc(rdbuff, buff_max);
			}
			fread(rdbuff, count, 1, fp);
			fclose(fp);
			GLuint shader = glCreateShader(stage);
			glShaderSource(shader, 1, (const char**)&rdbuff, &count);
			glCompileShader(shader);
			glGetShaderiv(shader, GL_COMPILE_STATUS, &count);
			if (!count)
			{
				//glGetShaderInfoLog(shader, buff_max, &count, rdbuff);
				glDeleteProgram(result);
				result = 0;
				break;
			}
			glAttachShader(result, shader);
			glDeleteShader(shader);
		}
	}
	glLinkProgram(result);
	glGetProgramiv(result, GL_LINK_STATUS, &count);
	if (!count)
	{
		//glGetProgramInfoLog(program, buff_max, &status, nullptr);
		glDeleteProgram(result);
		result = 0;
	}
	else
	{
		glValidateProgram(result);
		glGetProgramiv(result, GL_VALIDATE_STATUS, &count);
		if (!count)
		{
			//glGetProgramInfoLog(program, buff_max, &status, nullptr);
			glDeleteProgram(result);
			result = 0;
		}
	}
	free(rdbuff);
	return result;
}
