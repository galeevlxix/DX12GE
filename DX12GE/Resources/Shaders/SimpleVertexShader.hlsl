struct ConstantData
{
    matrix MVP;
};

ConstantBuffer<ConstantData> ModelViewProjectionCB : register(b0);

struct VertexShaderInput
{
    float3 Position : POSITION;
    float3 Color : COLOR;
};

struct VertexShaderOutput
{
    float4 Position : SV_Position;
    float3 Color : COLOR;
};

VertexShaderOutput main(VertexShaderInput IN)
{
    VertexShaderOutput OUT;
    OUT.Position = mul(ModelViewProjectionCB.MVP, float4(IN.Position, 1.0f));
    OUT.Color = IN.Color;
    return OUT;
}