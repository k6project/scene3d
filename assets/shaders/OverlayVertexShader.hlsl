static const float3 Vertex[] =
{
	float3(-1, 1, 0), float3(-1, -1, 0), float3(1, 1, 0), float3(1, -1, 0)
};

float4 OverlayVSMain( uint vertexId : SV_VERTEXID ) : SV_POSITION
{
	return float4(Vertex[vertexId], 1);
}
