#ifndef VERTEX_
#define VERTEX_

struct Vertex { vec4 attr[2]; };
#define Vertex_GetPosition(vi) (vi.attr[0].xyz)
#define Vertex_SetPosition(vi,v) (vi.attr[0].xyz = v.xyz)
#define Vertex_GetVertexColor(vi) (unpackUnorm4x8(floatBitsToUint(vi.attr[0].w)))
#define Vertex_SetVertexColor(vi,v) (vi.attr[0].w = uintBitsToFloat(packUnorm4x8(v)))
#define Vertex_GetNormal(vi) (vi.attr[1].xyz)
#define Vertex_SetNormal(vi,v) (vi.attr[1].xyz = v)

#endif // VERTEX_
