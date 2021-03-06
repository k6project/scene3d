#ifndef VERTEX_
#define VERTEX_

struct VBOData 
{ 
    vec4 attr[2];
};

struct VSInput
{
    vec3 position;
    vec4 color;
    vec3 normal;
};

void unpackVertex(in VBOData attrs, out VSInput vertex)
{
    vertex.position = attrs.attr[0].xyz;
    vertex.color = unpackUnorm4x8(floatBitsToUint(attrs.attr[0].w));
    vertex.normal = attrs.attr[1].xyz;
}

void packVertex(in VSInput vertex, out VBOData attrs)
{
    attrs.attr[0].xyz = vertex.position;
    attrs.attr[0].w = uintBitsToFloat(packUnorm4x8(vertex.color));
    attrs.attr[1].xyz = vertex.normal;
}

#ifdef STAGE_VS_

layout(location=0) in vec4 gAttr0;
layout(location=1) in vec4 gAttr1;

void vsMain(VSInput vertex);

void main()
{
    VSInput vertex;
    VBOData inVertex = {{gAttr0, gAttr1}};
    unpackVertex(inVertex, vertex);
    vsMain(vertex);
}

#endif // STAGE_VS_

#ifdef STAGE_CS_

#define VERTEX_DATA_BUFFER(n) buffer n##_ { VBOData n []; }

#endif // STAGE_CS_

#endif // VERTEX_
