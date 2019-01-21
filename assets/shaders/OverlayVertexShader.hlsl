static const float3 Vertex[] =
{
	float3(-1, 1, 0), float3(-1, -1, 0), float3(1, 1, 0), float3(1, -1, 0)
};

struct VSOutput
{
    float4 Position : SV_POSITION;
	float2 TexCoord : TEXCOORD_00;
};

VSOutput OverlayVSMain(uint vertexId : SV_VERTEXID)
{
    VSOutput output;
    output.Position = float4(0.5 * Vertex[vertexId], 1);
	output.TexCoord = (Vertex[vertexId].xy + float2(1, 1)) * 0.5;
	return output;
}
