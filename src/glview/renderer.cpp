#include "renderer.hpp"

#include <gfx/opengl.h>
#include <napp/filesys.h>

#include <vector>
#include <cstdio>

using namespace std;

struct gl_state_t
{
    vector<GLuint> shaders;
};

template<>
void renderer_impl<gl_state_t>::init() noexcept
{
}

template<>
id_t renderer_impl<gl_state_t>::load_material(const char *name) noexcept
{
    buffer_t rdbuff;
    buffer_init(&rdbuff, nullptr, 0);
    buffer_resize(rdbuff, 1024);
    char* str = buffer_data_ptr<char>(rdbuff, 0);
    snprintf(str, 1024, "%s.bmt", name);
    napp_fs_load_file(str, rdbuff);
    GLenum stages[NUM_SHADER_STAGES] =
    {
        GL_INVALID_ENUM, GL_INVALID_ENUM, GL_INVALID_ENUM,
        GL_INVALID_ENUM, GL_INVALID_ENUM
    };
    const char* fnames[NUM_SHADER_STAGES + 1] = {};
    for (int i = 0; i < NUM_SHADER_STAGES; i++)
    {
        if (int length = *str++)
        {
            switch (i)
            {
                case SHADER_STAGE_VERTEX:
                    stages[i] = GL_VERTEX_SHADER;
                    fnames[i] = str;
                    break;
                case SHADER_STAGE_FRAGMENT:
                    stages[i] = GL_FRAGMENT_SHADER;
                    fnames[i] = str;
                    break;
                default: break;
            }
            str += length;
        }
    }
    buffer_free(rdbuff);
    return ID_INVALID;
}

template<>
id_t renderer_impl<gl_state_t>::load_mesh(const char *name) noexcept
{
    return ID_INVALID;
}

template<>
void renderer_impl<gl_state_t>::begin_frame() noexcept
{
}

template<>
void renderer_impl<gl_state_t>::end_frame() noexcept
{
}

template<>
void renderer_impl<gl_state_t>::destroy() noexcept
{
}

template<>
void renderer_impl<gl_state_t>::draw(id_t material, id_t *mesh, usize_t count) noexcept
{
}

renderer_t& get_renderer()
{
    static renderer_impl<gl_state_t> impl;
    return impl;
}
