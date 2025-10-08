cbuffer OCB : register(b0)
{
    float4x4 MVP;
};

struct VSInput
{
    float3 Position : POSITION;
};

struct VSOutput
{
    float4 Position : SV_Position;
    float3 Direction : TEXCOORD0;
};

VSOutput main(VSInput IN)
{
    VSOutput OUT;    
    OUT.Position = mul(MVP, float4(IN.Position, 1.0f));
    OUT.Direction = IN.Position;
    return OUT;
}