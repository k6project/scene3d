#define STAGE_VS_

#include "vertex.inc"

layout(location=0) 
out VSOutput 
{ 
    vec4 normal;
    vec4 color; 
} gOut;

layout(set=0,binding=0)
uniform VSGlobal
{
    mat4 gViewProjection;
    mat4 gModelTransform;
    mat4 gNormalTransform;
};

void vsMain(VSInput vertex)
{
    gOut.normal = gNormalTransform * vec4(vertex.normal, 0.);
    gOut.color = vertex.color * vec4(abs(vertex.normal), 1.);
    gl_Position = vec4(vertex.position, 0.);
}
