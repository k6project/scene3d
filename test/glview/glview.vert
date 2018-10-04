#version 410 core

struct vertex_t
{
    vec3 pos;
    vec3 norm;
    vec2 uv0;
};

layout(location = 0) in vec4 g_vertex[2];

void unpack_vertex(out vertex_t vertex)
{
    vertex.pos = g_vertex[0].xyz;
    vertex.norm = vec3(g_vertex[0].w, g_vertex[1].xy);
    vertex.uv0 = g_vertex[1].zw;
}

out vs_out
{
    vec3 v_norm;
};

void main()
{
    vertex_t vs_in;
    unpack_vertex(vs_in);
    gl_Position = vec4(vs_in.pos, 1.0);
    v_norm = vs_in.norm; 
}
