#define STAGE_FS_

#include "fragment.inc"

layout(location=0)
in VSOutput 
{ 
    vec4 color; 
} gIn;

layout(location=0) out vec4 gColor0;

void fsMain()
{
    gColor0 = gIn.color;
}
