struct VSInput
{
    float3 Position : POSITION;
};

struct VSOutput
{
    float4 Position : SV_Position;
};

VSOutput main(VSInput IN)
{
    VSOutput OUT;
    OUT.Position = float4(IN.Position, 1.0f);
    return OUT;
}