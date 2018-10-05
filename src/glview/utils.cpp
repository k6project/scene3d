#include "utils.hpp"

#include <vector>

#include <napp/filesys.h>

struct bmd_t
{
    
    unsigned int v_count;
    int i_count;
    unsigned int v_bytes;
    unsigned int i_bytes;
    
    unsigned int vertex_stride() const noexcept
    {
        return (v_bytes / v_count);
    }
    
    unsigned int num_attrs() const noexcept
    {
        return (vertex_stride() >> 4u);
    }
    
    float* vertex_data() noexcept
    {
        return reinterpret_cast<float*>(this + 1);
    }
    
    unsigned int* index_data() noexcept
    {
        return reinterpret_cast<unsigned int*>(reinterpret_cast<char*>(this + 1) + v_bytes);
    }
    
};

void renderer::init() noexcept
{
    glCreateContextNAPP();
    glClearColor(0.4f, 0.6f, 1.0f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT);
}

void renderer::load_material(const std::string &name, bool set_current) noexcept
{
    string vert_fname = name + ".vert";
    string frag_fname = name + ".frag";
    GLenum stage_type[] = { GL_VERTEX_SHADER, GL_FRAGMENT_SHADER };
    const char* stage_fname[] = { vert_fname.c_str(), frag_fname.c_str(), nullptr };
    GLuint shader = glCreateShaderProgramNAPP(stage_fname, stage_type);
    materials_.emplace_back();
    materials_.back().shader = shader;
    if (set_current)
    {
        glUseProgram(shader);
    }
}

void renderer::load_meshes(const initializer_list<string>& args) noexcept
{
    mesh_id mid = 0;
    vector<buffer_t> buffers;
    buffers.reserve(args.size());
    meshes_.resize(args.size());
    mesh_map_.reserve(args.size());
	for (const string& str : args)
	{
        buffer_t rdbuff;
        buffer_init(&rdbuff, nullptr, 0);
        string fname = str + ".bmd";
        napp_fs_load_file(fname.c_str(), rdbuff);
        buffers.push_back(rdbuff);
        mesh_map_.emplace(str, mid++);
	}
    unsigned int v_bytes = 0, i_bytes = 0;
    for (unsigned int i = 0, i_offset = 0; i < buffers.size(); i++)
    {
        mesh_t& mesh = meshes_[i];
        bmd_t& bmd = buffer_data_ref<bmd_t>(buffers[i]);
        mesh.count = bmd.i_count;
        mesh.offset = reinterpret_cast<const GLvoid*>(i_offset * sizeof(GLuint));
        v_bytes += bmd.v_bytes;
        i_bytes += bmd.i_bytes;
        unsigned int* indices = bmd.index_data();
        for (int i = 0; i < bmd.i_count; i++)
        {
            indices[i] += i_offset;
        }
        i_offset += static_cast<unsigned int>(bmd.i_count);
    }
    for (unsigned int i = 0, v_offset = 0, i_offset = 0; i < buffers.size(); i++)
    {
        bmd_t& bmd = buffer_data_ref<bmd_t>(buffers[i]);
        if(!vao_)
        {
            glGenVertexArrays(1, &vao_);
            glBindVertexArray(vao_);
            glGenBuffers(2, buff_.ptr);
            glBindBuffer(GL_ARRAY_BUFFER, buff_.vbo);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buff_.ibo);
            glBufferData(GL_ARRAY_BUFFER, v_bytes, nullptr, GL_STATIC_DRAW);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, i_bytes, nullptr, GL_STATIC_DRAW);
            GLsizei stride = bmd.vertex_stride();
            for (unsigned int i = 0; i < bmd.num_attrs(); i++)
            {
                glEnableVertexAttribArray(i);
                auto ptr = reinterpret_cast<const GLvoid*>(i * (sizeof(float) << 2));
                glVertexAttribPointer(i, 4, GL_FLOAT, GL_FALSE, stride, ptr);
            }
        }
        glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, i_offset, bmd.i_bytes, bmd.index_data());
        glBufferSubData(GL_ARRAY_BUFFER, v_offset, bmd.v_bytes, bmd.vertex_data());
        v_offset += bmd.v_bytes;
        i_offset += bmd.i_bytes;
        buffer_free(buffers[i]);
    }
}

renderer::mesh_id renderer::get_mesh(const string &name) noexcept
{
    return mesh_map_[name];
}

void renderer::begin_frame() noexcept
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}

void renderer::end_frame() noexcept
{
    glSwapBuffersNAPP();
}

void renderer::draw_mesh(mesh_id id) noexcept
{
    const mesh_t& mesh = meshes_[id];
    glDrawElements(GL_TRIANGLES, mesh.count, GL_UNSIGNED_INT, mesh.offset);
}

void renderer::destroy() noexcept
{
    glUseProgram(0);
    for (const material_t& mat : materials_)
    {
        glDeleteProgram(mat.shader);
    }
    glBindVertexArray(0);
    glDeleteVertexArrays(1, &vao_);
    glDeleteBuffers(2, buff_.ptr);
    glDestroyContextNAPP();
}
