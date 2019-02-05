cbuffer GlobalParameters : register(b0)
{
    float4x4 Projection;
    float4x4 ViewTransform;
}

static const float3 Vertex[] =
{
	float3(-1,  1,  0), 
    float3(-1, -1,  0), 
    float3( 1,  1,  0), 
    float3( 1, -1,  0)
};

struct VSOutput
{
    float4 Position : SV_POSITION;
	float2 TexCoord : TEXCOORD_00;
};

VSOutput OverlayVSMain(uint vertexId : SV_VERTEXID)
{
    VSOutput output;
    float4 localPosition = float4(Vertex[vertexId], 1);
    float4 viewPosition = mul(ViewTransform, localPosition);
    output.Position = mul(Projection, viewPosition);
	output.TexCoord = (Vertex[vertexId].xy + float2(1, 1)) * 0.5;
	return output;
}
