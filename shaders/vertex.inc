#ifndef VERTEX_
#define VERTEX_

struct Vertex { vec4 attr[2]; };
#define Vertex_GetPosition(vi) (vi.attr[0].xyz)
#define Vertex_SetPosition(vi,v) (vi.attr[0].xyz = v.xyz)
#define Vertex_GetVertexColor(vi) (vi.attr[1].rgb)
#define Vertex_SetVertexColor(vi,v) (vi.attr[1].rgb = v.rgb)

#endif // VERTEX_
