struct ConstantData
{
    matrix WVP;
};

ConstantBuffer<ConstantData> WorldViewProjectionCB : register(b0);

struct VertexShaderInput
{
    float3 Position : POSITION;
    float2 TextCoord : TEXCOORD;
};

struct VertexShaderOutput
{
    float4 Position : SV_Position;
    float2 TextCoord : TEXCOORD;
};

VertexShaderOutput main(VertexShaderInput IN)
{
    VertexShaderOutput OUT;
    OUT.Position = mul(WorldViewProjectionCB.WVP, float4(IN.Position, 1.0f));
    OUT.TextCoord = IN.TextCoord;
    return OUT;
}