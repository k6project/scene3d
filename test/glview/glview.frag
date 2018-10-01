#version 410 core

in vs_out
{
    vec3 v_norm;
};

layout(location = 0) out vec4 color0;

void main()
{
    color0 = vec4(abs(v_norm), 1.0);
}
