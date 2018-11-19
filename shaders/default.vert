#define STAGE_VS_

#include "vertex.inc"

layout(location=0) out VSOutput { vec4 color; }gVsOut;

void vsMain(VSInput vertex)
{
    gVsOut.color = vertex.color;
    gl_Position = vec4(vertex.position, 0.);
}