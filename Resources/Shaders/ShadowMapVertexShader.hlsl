struct ConstantData
{
    matrix MVP;
};

ConstantBuffer<ConstantData> ModelViewProjectionCB : register(b0);

struct VertexPosColor
{
    float3 Position : POSITION;
};

struct VertexShaderOutput
{
    float4 Position : SV_Position;
};

VertexShaderOutput main(VertexPosColor IN)
{
    VertexShaderOutput OUT;
    OUT.Position = mul(ModelViewProjectionCB.MVP, float4(IN.Position, 1.0f));
    return OUT;
}