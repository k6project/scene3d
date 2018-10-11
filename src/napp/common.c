#include "common.h"

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <gfx/opengl.h>
#include <gfx/vulkan.h>
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

void* buffer_data(buffer_t buffer, unsigned int offset)
{
    return (((char*)buffer->ptr) + offset);
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

int napp_fs_load_file(const char* fname, buffer_t rdbuff)
{
    int bytes = -1;
    FILE* fp = fopen(fname, "rb");
    if (fp)
    {
        fseek(fp, 0, SEEK_END);
        bytes = ftell(fp) & INT_MAX;
        fseek(fp, 0, SEEK_SET);
        if (buffer_resize(rdbuff, bytes) >= bytes)
        {
            fread(buffer_data(rdbuff, 0), bytes, 1, fp);
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
    buffer_t rdbuff;
	GLuint result = 0;
    buffer_init(&rdbuff, NULL, 4096);
	result = glCreateProgram();
	for (int i = 0; files[i]; i++)
	{
		GLenum stage = stages[i];
        if (stage == GL_INVALID_ENUM)
        {
            continue;
        }
		const char* fname = files[i];
        count = napp_fs_load_file(fname, rdbuff);
        if (count > 0)
        {
			GLuint shader = glCreateShader(stage);
            const char* src = buffer_data(rdbuff, 0);
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
	return result;
}

VkResult vkInitFunctionsNAPP(VkInstance instance, VkDevice device)
{
    static PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr = NULL;
    if (instance == VK_NULL_HANDLE)
    {
        if (device != VK_NULL_HANDLE)
        {
            // init device-level
        }
        else
        {
            //load dll, get pointer to vkGetInstanceProcAddr
            //init global-level
        }
    }
    else if (device == VK_NULL_HANDLE)
    {
        //init instance-level
    }
    else
    {
        return VK_ERROR_INITIALIZATION_FAILED;
    }
    return VK_SUCCESS;
}

VkResult vkCreateInstanceNAPP(VkInstance* instance)
{
    VkInstance inst = VK_NULL_HANDLE;
    vkInitFunctionsNAPP(VK_NULL_HANDLE, VK_NULL_HANDLE);
    //perform instance creation
    vkInitFunctionsNAPP(inst, VK_NULL_HANDLE);
    *instance = inst;
    return VK_SUCCESS;
}
