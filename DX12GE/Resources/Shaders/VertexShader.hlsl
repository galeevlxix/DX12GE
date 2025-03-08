struct WorldViewProjection
{
    matrix MVP;
    matrix WVP;
};

ConstantBuffer<WorldViewProjection> WorldViewProjectionCB : register(b0);

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
};

VertexShaderOutput main(VertexPosColor IN)
{
    VertexShaderOutput OUT;

    OUT.Position = mul(WorldViewProjectionCB.WVP, float4(IN.Position, 1.0f));
    OUT.Normal = normalize(mul(WorldViewProjectionCB.MVP, float4(IN.Normal, 0.0f)));
    OUT.TextCoord = IN.TextCoord;

    return OUT;
}