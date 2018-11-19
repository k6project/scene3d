#define STAGE_VS_

#include "vertex.inc"

layout(location=0) 
out VSOutput 
{ 
    vec4 color; 
} gOut;

void vsMain(VSInput vertex)
{
    gOut.color = vertex.color;
    gl_Position = vec4(vertex.position, 0.);
}