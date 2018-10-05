#pragma once

#include <gfx/opengl.h>

#include <string>
#include <vector>
#include <unordered_map>
#include <initializer_list>

using namespace std;

class renderer
{
public:
    typedef unsigned int mesh_id;
    typedef unsigned int material_id;
    void init() noexcept;
    void load_material(const string& name, bool set_current) noexcept;
	void load_meshes(const initializer_list<string>& args) noexcept;
    mesh_id get_mesh(const string& name) noexcept;
    void begin_frame() noexcept;
    void end_frame() noexcept;
    void draw_mesh(mesh_id id) noexcept;
    void destroy() noexcept;
private:
    struct mesh_t
    {
        GLsizei count;
        const GLvoid* offset;
    };
    
    struct material_t
    {
        GLuint shader;
    };
    
    vector<mesh_t> meshes_;
    vector<material_t> materials_;
    unordered_map<string, mesh_id> mesh_map_;
    
    GLuint vao_ = 0;
    union
    {
        struct
        {
            GLuint vbo, ibo;
        };
        GLuint ptr[2];
    } buff_;
};
