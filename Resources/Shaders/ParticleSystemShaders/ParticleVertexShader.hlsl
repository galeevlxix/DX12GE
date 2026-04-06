struct VSInput
{
    float3 Position : POSITION;
    float3 Velocity : VELOCITY;
};

struct VSOutput
{
    float4 Position : SV_Position;
    float4 Velocity : VELOCITY;
};

VSOutput main(VSInput IN)
{
    VSOutput OUT;
    OUT.Position = float4(IN.Position, 1.0f);
    OUT.Velocity = float4(IN.Velocity, 1.0f);
    return OUT;
}