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

static const float2 Vertex[] =
{
	float2(-1,  1), float2(-1, -1), float2(1,  1),
	float2(1,  1), float2(-1, -1), float2(1, -1)
};

float4 UnpackColor(uint input)
{
	float4 v = float4(
		float(input >> 24), 
		float((input >> 16) & 255), 
		float((input >> 8) & 255), 
		float(input & 255) ) * 0.00392156862745098;
	return saturate(v);
}

VSOutput TileMapVSMain(uint vertexId : SV_VERTEXID)
{
	VSOutput output;
	uint index = vertexId % 6;
	float4 localPosition = float4(Vertex[index], 0, 1);
	float4 worldPosition = mul(ModelTransform, localPosition);
	float4 viewPosition = mul(ViewTransform, worldPosition);
	output.Position = mul(Projection, viewPosition);
	output.FaceColor = UnpackColor(asuint(FaceNormalAndColor.w));
	output.TexCoord = (Vertex[index] + float2(1, 1)) * 0.5;
	return output;
}
