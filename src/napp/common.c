#include "common.h"

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <gfx/opengl.h>
#include <napp/filesys.h>

struct buffer_t_
{
    void* ptr;
    int size;
};

void buffer_init(buffer_t* buffer, void* memory, int size)
{
    *buffer = (buffer_t)calloc(1, sizeof(struct buffer_t_));
}

int buffer_resize(buffer_t buffer, int new_size)
{
    if (!buffer->ptr || buffer->size < new_size)
    {
        buffer->ptr = realloc(buffer->ptr, new_size);
        buffer->size = new_size;
    }
    return buffer->size;
}

void* buffer_data(buffer_t buffer)
{
    return buffer->ptr;
}

void buffer_free(buffer_t buffer)
{
    free(buffer->ptr);
    free(buffer);
}

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

//int napp_fs_load_file(const char* fname, void** rdbuff, int* max)
int napp_fs_load_file(const char* fname, buffer_t rdbuff)
{
    int bytes = -1;
    FILE* fp = fopen(fname, "rb");
    if (fp)
    {
        fseek(fp, 0, SEEK_END);
        bytes = ftell(fp) & INT_MAX;
        fseek(fp, 0, SEEK_SET);
        /*if (!*rdbuff || bytes > *max)
        {
            if (bytes > *max)
            {
                *max = bytes;
            }
            *rdbuff = malloc(*max);
        }
        if (*rdbuff && *max >= bytes)*/
        if (buffer_resize(rdbuff, bytes) >= bytes)
        {
            fread(buffer_data(rdbuff), bytes, 1, fp);
        }
        else
        {
            bytes = -2;
        }
        fclose(fp);
    }
    return bytes;
}

GLuint glCreateShaderProgramNAPP(const char** files, GLenum* stages)
{
    int count = 0;
	GLuint result = 0;
	//void* rdbuff = NULL;
	//int buff_max = 4096, count;
    buffer_t rdbuff;
    buffer_init(&rdbuff, NULL, 4096);
	result = glCreateProgram();
	for (int i = 0; files[i]; i++)
	{
		GLenum stage = stages[i];
		const char* fname = files[i];
        //count = napp_fs_load_file(fname, &rdbuff, &buff_max);
        count = napp_fs_load_file(fname, rdbuff);
        if (count > 0)
        {
			GLuint shader = glCreateShader(stage);
            const char* src = buffer_data(rdbuff);
			glShaderSource(shader, 1, &src, &count);
			glCompileShader(shader);
			glGetShaderiv(shader, GL_COMPILE_STATUS, &count);
			if (!count)
			{
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
		glDeleteProgram(result);
		result = 0;
	}
    buffer_free(rdbuff);
	//free(rdbuff);
	return result;
}
