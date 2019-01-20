struct VSOutput
{
    float4 Position : SV_POSITION;
    float4 Color    : VERTEXCOLOR;
};

float4 PSMain(VSOutput input) : SV_TARGET0
{
    return input.Color;
}
