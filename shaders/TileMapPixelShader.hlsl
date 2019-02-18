cbuffer GlobalParameters : register(b0)
{
	float4x4 Projection;
	float4x4 ViewTransform;
}

cbuffer LocalParameters : register(b1)
{
	float4x4 ModelTransform;
	float4 FaceNormalAndColor;
}

struct VSOutput
{
	float4 Position : SV_POSITION;
	float4 FaceColor: VERTEXCOLOR;
	float2 TexCoord : TEXCOORD_00;
};

float4 TileMapPSMain(VSOutput input) : SV_TARGET0
{
	return input.FaceColor;
}
