#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <par/par_shapes.h>
#include <hmm/HandmadeMath.h>

void print_db(const par_shapes_mesh* mesh, const bool do_filter)
{
	hmm_vec3* vertices = reinterpret_cast<hmm_vec3*>(mesh->points);
	hmm_vec3* normals = reinterpret_cast<hmm_vec3*>(mesh->normals);
	struct tmp_t { hmm_vec3 v, n; int prev, idx; } *filter = nullptr;
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
				//printf("Add vertex %d\n", a.idx);
				for (int j = i + 1; j < mesh->npoints; j++)
				{
					tmp_t& b = filter[j];
					const hmm_vec3 dv = HMM_SubtractVec3(a.v, b.v);
					const hmm_vec3 dn = HMM_SubtractVec3(a.n, b.n);
					if (HMM_DotVec3(dv, dv) < 0.000001f && HMM_DotVec3(dn, dn) < 0.000001f)
					{
						//printf("    Remap vertex %d to %d\n", j, a.idx);
						b.prev = i;
						b.idx = a.idx;
						--unique;
					}
				}
			}
		}
	}
	char format[4] = { '-', '-', '-', '\0' };
	if (mesh->points) format[0] = '+';
	if (mesh->normals) format[1] = '+';
	if (mesh->tcoords) format[2] = '+';
	int bpi = (unique > 256) ? ((unique > 65535) ? 4 : 2) : 1;
	printf("%d %d %d %s\n", unique, mesh->ntriangles, bpi, format);
	for (int i = 0; i < mesh->npoints; i++)
	{
		const tmp_t& a = filter[i];
		if (a.prev == -1)
		{
			printf("%0.4f %0.4f %0.4f %0.4f %0.4f %0.4f\n",
				a.v.X, a.v.Y, a.v.Z, a.n.X, a.n.Y, a.n.Z);
		}
	}
	for (int i = 0; i < mesh->ntriangles * 3;)
	{
		const tmp_t& a = filter[mesh->triangles[i++]];
		const tmp_t& b = filter[mesh->triangles[i++]];
		const tmp_t& c = filter[mesh->triangles[i++]];
		printf("%d %d %d\n", a.idx, b.idx, c.idx);
	}
	free(filter);
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
		print_db(mesh, filter);
		par_shapes_free_mesh(mesh);
	}
	return 0;
}
