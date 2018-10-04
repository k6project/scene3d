#include "utils.hpp"

#include <vector>

#include <napp/filesys.h>

struct bmd_t
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
};

struct bmd_t_
{
    
    unsigned int v_count;
    unsigned int i_count;
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

/*
 GLsizei stride = bmd.vertex_stride();
 for (unsigned int i = 0; i < bmd.num_attrs(); i++)
 {
     glEnableVertexAttribArray(i);
     auto ptr = reinterpret_cast<const GLvoid*>(i * (sizeof(float) << 2));
     glVertexAttrbPointer(i, 4, GL_FLOAT, GL_FALSE, stride, ptr);
 }
 */

void mesh_lib::init(const initializer_list<string>& args) noexcept
{
    vector<buffer_t> buffers;
    buffers.reserve(args.size());
	for (const string& str : args)
	{
        buffer_t rdbuff;
        buffer_init(&rdbuff, nullptr, 0);
        string fname = str + ".bmd";
        napp_fs_load_file(fname.c_str(), rdbuff);
        buffers.push_back(rdbuff);
	}
    size_t v_bytes = 0, i_bytes = 0;
    auto accum_size = [&](buffer_t buffer)
    {
        const bmd_t& bmd = buffer_data_ref<bmd_t>(buffer);
        v_bytes += ALIGNED(bmd.v_bytes, 16);
        i_bytes += ALIGNED(bmd.i_bytes, 16);
    };
    for_each(buffers.begin(), buffers.end(), accum_size);
    auto mesh_upload = [&](buffer_t buffer)
    {
        buffer_free(buffer);
    };
    for_each(buffers.begin(), buffers.end(), mesh_upload);
}
