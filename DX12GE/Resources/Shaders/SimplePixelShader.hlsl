struct PixelShaderInput
{
    float4 Position : SV_Position;
    float3 Color : COLOR;
};

float4 main(PixelShaderInput IN) : SV_Target
{
    return float4(IN.Color, 1.0);
}