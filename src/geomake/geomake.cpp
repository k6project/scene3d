#include <cstdio>
#include <cstdlib>
#include <cstring>

#define PAR_SHAPES_IMPLEMENTATION

#include <par/par_shapes.h>
#include <hmm/HandmadeMath.h>

#include "writers.hpp"

void present_mesh(const par_shapes_mesh* mesh, const bool do_filter = false, mesh_writer& out = default_writer())
{
	hmm_vec3* vertices = reinterpret_cast<hmm_vec3*>(mesh->points);
	hmm_vec3* normals = reinterpret_cast<hmm_vec3*>(mesh->normals);
	struct tmp_t { hmm_vec3 v, n; hmm_vec2 tc; int prev, idx; } *filter = nullptr;
	filter = (tmp_t*)calloc(mesh->npoints, sizeof(tmp_t));
	for (int i = 0; i < mesh->npoints; i++)
	{
		filter[i].v = vertices[i];
		filter[i].n = normals[i];
		filter[i].prev = -1;
		filter[i].idx = (do_filter) ? -1 : i;
	}
	int unique = mesh->npoints, next = 0;

	if (do_filter)
	{
		for (int i = 0; i < mesh->npoints; i++)
		{
			if (filter[i].prev == -1)
			{
				filter[i].idx = next++;
				const tmp_t& a = filter[i];
				for (int j = i + 1; j < mesh->npoints; j++)
				{
					tmp_t& b = filter[j];
					const hmm_vec3 dv = HMM_SubtractVec3(a.v, b.v);
					const hmm_vec3 dn = HMM_SubtractVec3(a.n, b.n);
					if (HMM_DotVec3(dv, dv) < 0.000001f && HMM_DotVec3(dn, dn) < 0.000001f)
					{
						b.prev = i;
						b.idx = a.idx;
						--unique;
					}
				}
			}
		}
	}

	out.begin_mesh(unique, mesh->ntriangles, (mesh->normals) != nullptr, (mesh->tcoords) != nullptr);
	out.begin_vertices(unique);
	for (int i = 0; i < mesh->npoints; i++)
	{
		const tmp_t& a = filter[i];
		if (a.prev == -1)
		{
			out.begin_vertex();
			out.vertex_position(a.v);
			if (mesh->normals)
			{
				out.vertex_normal(a.n);
			}
			if (mesh->tcoords)
			{
				out.vertex_texcoord(a.tc);
			}
			out.end_vertex();
		}
	}
	out.end_vertices(unique);
	out.begin_triangles(mesh->ntriangles);
	for (int i = 0; i < mesh->ntriangles * 3;)
	{
		const tmp_t& a = filter[mesh->triangles[i++]];
		const tmp_t& b = filter[mesh->triangles[i++]];
		const tmp_t& c = filter[mesh->triangles[i++]];
		out.begin_triangle();
		out.triangle_indices(a.idx, b.idx, c.idx);
		out.end_triangle();
	}
	out.end_triangles(mesh->ntriangles);
	out.end_mesh();

	free(filter);
}

template <typename T, T* obj>
void cb(void* arg)
{
	obj->test();
}

int main(int argc, char** argv)
{
	if (argc > 1)
	{
		bool filter = false;
		const char* arg = argv[1];
		par_shapes_mesh* mesh = nullptr;
		if (*arg == 'p')
		{
			if (strcmp(arg, "p4") == 0)
				mesh = par_shapes_create_tetrahedron();
			else if (strcmp(arg, "p6") == 0)
				mesh = par_shapes_create_cube();
			else if (strcmp(arg, "p8") == 0)
				mesh = par_shapes_create_octahedron();
			else if (strcmp(arg, "p12") == 0)
				mesh = par_shapes_create_dodecahedron();
			else if (strcmp(arg, "p20") == 0)
				mesh = par_shapes_create_icosahedron();
			par_shapes_unweld(mesh, true);
			par_shapes_compute_normals(mesh);
			filter = true;
		}
		present_mesh(mesh, filter, json_writer());
		par_shapes_free_mesh(mesh);
	}
	return 0;
}
