cbuffer OCB : register(b0)
{
    float4x4 WVP;
    float4x4 MVP;
};

cbuffer SCB : register(b1)
{
    matrix ShadowMapTransform0;
    matrix ShadowMapTransform1;
    matrix ShadowMapTransform2;
};

struct VertexPosColor
{
    float3 Position : POSITION;
    float3 Normal : NORMAL;
    float2 TextCoord : TEXCOORD;
};

struct VertexShaderOutput
{
    float4 Position : SV_Position;
    float4 Normal : NORMAL;
    float2 TextCoord : TEXCOORD;
    float4 ShadowPos0 : POSITION0;
    float4 ShadowPos1 : POSITION1;
    float4 ShadowPos2 : POSITION2;
    float4 WorldPos : POSITION4;
};

VertexShaderOutput main(VertexPosColor IN)
{
    VertexShaderOutput OUT;

    OUT.Position = mul(MVP, float4(IN.Position, 1.0f));
    OUT.Normal = normalize(mul(WVP, float4(IN.Normal, 0.0f)));
    OUT.TextCoord = IN.TextCoord;
    
    OUT.ShadowPos0 = mul(ShadowMapTransform0, float4(IN.Position, 1.0f));
    OUT.ShadowPos1 = mul(ShadowMapTransform1, float4(IN.Position, 1.0f));
    OUT.ShadowPos2 = mul(ShadowMapTransform2, float4(IN.Position, 1.0f));
    
    OUT.WorldPos = mul(WVP, float4(IN.Position, 1.0f));
    
    return OUT;
}