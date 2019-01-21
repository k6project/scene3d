struct VSOutput
{
    float4 Position : SV_POSITION;
	float2 TexCoord : TEXCOORD_00;
};

static const float4 Color[] =
{
	float4(1, 0, 0, 1), float4(0, 0, 0, 1), float4(0, 1, 0, 1), float4(0, 0, 1, 1)
};

float4 PSMain(VSOutput input) : SV_TARGET0
{
	float4 color = lerp(lerp(Color[0], Color[1], input.TexCoord.x), lerp(Color[2], Color[3], input.TexCoord.x), input.TexCoord.y);
	return color;
}
