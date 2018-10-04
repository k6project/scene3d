#pragma once

#include <cstdio>

union hmm_vec2;
union hmm_vec3;

struct mesh_writer
{
	virtual void begin_mesh(int verts, int tris, bool norm, bool tc) {}
	virtual void begin_vertices(int count) {}
	virtual void begin_vertex() {}
	virtual void vertex_position(const hmm_vec3& val) {}
	virtual void vertex_normal(const hmm_vec3& val) {}
	virtual void vertex_texcoord(const hmm_vec2& val) {}
	virtual void end_vertex() {}
	virtual void end_vertices(int count) {}
	virtual void begin_triangles(int count) {}
	virtual void begin_triangle() {}
	virtual void triangle_indices(int a, int b, int c) {}
	virtual void end_triangle() {}
	virtual void end_triangles(int count) {}
	virtual void end_mesh() {}
};

struct default_writer : public mesh_writer
{
	default_writer();
    void set_stream(FILE* fp);
	virtual void begin_mesh(int verts, int tris, bool norm, bool tc) override;
	virtual void vertex_position(const hmm_vec3& val) override;
	virtual void vertex_normal(const hmm_vec3& val) override;
	virtual void vertex_texcoord(const hmm_vec2& val) override;
	virtual void end_vertex() override;
	virtual void triangle_indices(int a, int b, int c) override;
protected:
	FILE* stream = nullptr;
};

//todo: pack attributes into 4-component float vectors
//header: 4b-num verts, 4b-num inds, 4b-size vdata, 4b-size idata
//mesh has only 1 attribute - array of 4-component vectors
struct json_writer : public default_writer
{
	virtual void begin_mesh(int verts, int tris, bool norm, bool tc) override;
	virtual void begin_vertices(int count) override;
	virtual void begin_vertex() override;
	virtual void vertex_position(const hmm_vec3& val) override;
	virtual void vertex_normal(const hmm_vec3& val) override;
	virtual void vertex_texcoord(const hmm_vec2& val) override;
	virtual void end_vertex() override;
	virtual void end_vertices(int count) override;
	virtual void begin_triangles(int count) override;
	virtual void begin_triangle() override;
	virtual void triangle_indices(int a, int b, int c) override;
	virtual void end_triangle() override;
	virtual void end_triangles(int count) override;
	virtual void end_mesh() override;
protected:
	int counter = 0;
};

struct binary_writer : public default_writer
{
    virtual void begin_mesh(int verts, int tris, bool norm, bool tc) override;
    virtual void vertex_position(const hmm_vec3& val) override;
    virtual void vertex_normal(const hmm_vec3& val) override;
    virtual void vertex_texcoord(const hmm_vec2& val) override;
    virtual void end_vertex() override;
    virtual void triangle_indices(int a, int b, int c) override;
protected:
    int stride = sizeof(float) * (3 + 3 + 2);
    bool has_norm = false, has_tc = false;
};
