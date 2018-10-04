#include "writers.hpp"

#include <hmm/HandmadeMath.h>

/****                           DEFAULT WRITER                            ****/

default_writer::default_writer()
	: stream(stdout)
{
}

void default_writer::set_stream(FILE *fp)
{
    stream = (fp) ? fp : stdout;
}

void default_writer::begin_mesh(int verts, int tris, bool norm, bool tc)
{
	char format[4] = { '+', '-', '-', '\0' };
	if (norm) format[1] = '+';
	if (tc) format[2] = '+';
	int bpi = (verts > 256) ? ((verts > 65535) ? 4 : 2) : 1;
	fprintf(stream, "%d %d %d %s\n", verts, tris, bpi, format);
}

void default_writer::vertex_position(const hmm_vec3& val)
{
	fprintf(stream, "%0.4f %0.4f %0.4f", val.X, val.Y, val.Z);
}

void default_writer::vertex_normal(const hmm_vec3& val)
{
	fprintf(stream, " %0.4f %0.4f %0.4f", val.X, val.Y, val.Z);
}

void default_writer::vertex_texcoord(const hmm_vec2& val)
{
	fprintf(stream, " %0.4f %0.4f", val.X, val.Y);
}

void default_writer::end_vertex()
{
	putc('\n', stream);
}

void default_writer::triangle_indices(int a, int b, int c)
{
	fprintf(stream, "%d %d %d\n", a, b, c);
}

/****                            JSON WRITER                              ****/

void json_writer::begin_mesh(int verts, int tris, bool norm, bool tc)
{
	int bpi = (verts > 256) ? ((verts > 65535) ? 4 : 2) : 1;
	fprintf(stream, 
		"{\n"
		"    \"num_vertices\"  : %d,\n"
	    "    \"num_triangles\" : %d,\n"
		"    \"index_bytes\"   : %d,\n"
		"    \"has_normals\"   : %s,\n"
		"    \"has_tcoords\"   : %s", 
		verts, tris, bpi, 
		(norm) ? "true" : "false", 
		(tc) ? "true" : "false");
}

void json_writer::begin_vertices(int count)
{
	counter = count;
	if (count > 0) fprintf(stream, ",\n    \"vertices\" :\n    [");
	putc('\n', stream);
}

void json_writer::begin_vertex()
{
	fprintf(stream, "        [ ");
}

void json_writer::vertex_position(const hmm_vec3& val)
{
	fprintf(stream, "[%0.4f,%0.4f,%0.4f]", val.X, val.Y, val.Z);
}

void json_writer::vertex_normal(const hmm_vec3& val)
{
	fprintf(stream, ", [%0.4f,%0.4f,%0.4f]", val.X, val.Y, val.Z);
}

void json_writer::vertex_texcoord(const hmm_vec2& val)
{
	fprintf(stream, ", [%0.4f,%0.4f]", val.X, val.Y);
}

void json_writer::end_vertex()
{
	fprintf(stream, " ]");
	if (--counter > 0)
	{
		putc(',', stream);
	}
	putc('\n', stream);
}

void json_writer::end_vertices(int count)
{
	if (count > 0) fprintf(stream, "    ]");
}

void json_writer::begin_triangles(int count)
{
	counter = count;
	if (count > 0) fprintf(stream, ",\n    \"triangles\" :\n    [");
	putc('\n', stream);
}

void json_writer::begin_triangle()
{
}

void json_writer::triangle_indices(int a, int b, int c)
{
	fprintf(stream, "        %d, %d, %d", a, b, c);
}

void json_writer::end_triangle()
{
	if (--counter > 0)
	{
		putc(',', stream);
	}
	putc('\n', stream);
}

void json_writer::end_triangles(int count)
{
	if (count > 0) fprintf(stream, "    ]");
	putc('\n', stream);
}

void json_writer::end_mesh()
{
	putc('}', stream);
	putc('\n', stream);
}

/****                           BINARY WRITER                             ****/

template <typename T>
static void write_value(FILE* stream, const T& v, int count = 1)
{
    fwrite(&v, sizeof(T), count, stream);
}

void binary_writer::begin_mesh(int verts, int tris, bool norm, bool tc)
{
#if 0
    write_value(stream, verts);
    int indices = 3 * tris;
    write_value(stream, indices);
    char attr_idx = 0;
    char offs = 0;
    char stride = 3 * sizeof(float);
    stride += (norm) ? 3 * sizeof(float) : 0;
    stride += (tc) ? 2 * sizeof(float) : 0;
    struct attr_t { char idx, size, stride, offs; };
    attr_t pos { attr_idx++, 3, stride, 0 };
    write_value(stream, pos);
    offs += 3 * sizeof(float);
    {
        char i = (norm) ? attr_idx++ : -1;
        attr_t anorm { i, 3, stride, offs };
        write_value(stream, anorm);
        offs += 3 * sizeof(float);
    }
    {
        char i = (tc) ? attr_idx++ : -1;
        attr_t atc { i, 2, stride, offs };
        write_value(stream, atc);
        offs += 2 * sizeof(float);
    }
    int vdata_size = stride * verts;
    write_value(stream, vdata_size);
    int idata_offs = 16 * (((vdata_size - 1) / 16) + 1);
    write_value(stream, idata_offs);
    pad = idata_offs - vdata_size;
    int bpis = 4 * indices;
    write_value(stream, bpis);
#else
    has_tc = tc;
    has_norm = norm;
    int indices = 3 * tris;
    write_value(stream, verts);
    write_value(stream, indices);
    write_value(stream, stride * verts);
    write_value(stream, indices * 4);
#endif
}

void binary_writer::vertex_position(const hmm_vec3& val)
{
    static hmm_vec3 zero;
    write_value(stream, val);
    if (!has_norm)
    {
        write_value(stream, zero);
    }
}

void binary_writer::vertex_normal(const hmm_vec3& val)
{
    write_value(stream, val);
}

void binary_writer::vertex_texcoord(const hmm_vec2& val)
{
    write_value(stream, val);
}

void binary_writer::end_vertex()
{
    static hmm_vec2 zero;
    if (!has_tc)
    {
        write_value(stream, zero);
    }
}

void binary_writer::triangle_indices(int a, int b, int c)
{
    int indices[3] = {a, b, c};
    write_value(stream, indices);
}
