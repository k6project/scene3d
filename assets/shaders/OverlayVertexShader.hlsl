static const float3 Vertex[] =
{
	float3(-1, 1, 0), float3(-1, -1, 0), float3(1, 1, 0), float3(1, -1, 0)
};

static const float4 Color[] =
{
    float4(1, 0, 0, 1), float4(1, 1, 0, 1), float4(0, 1, 0, 1), float4(0, 0, 1, 1)
};

struct VSOutput
{
    float4 Position : SV_POSITION;
    float4 Color    : VERTEXCOLOR;
};

VSOutput OverlayVSMain(uint vertexId : SV_VERTEXID)
{
    VSOutput output;
    output.Position = float4(Vertex[vertexId], 1);
    output.Color = Color[vertexId];
	return output;
}
