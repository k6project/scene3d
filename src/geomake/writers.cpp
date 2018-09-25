#include "writers.hpp"

#include <hmm/HandmadeMath.h>

/****                           DEFAULT WRITER                            ****/

default_writer::default_writer()
	: stream(stdout)
{
}

default_writer::default_writer(FILE* fstream)
	: stream(fstream)
{
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
