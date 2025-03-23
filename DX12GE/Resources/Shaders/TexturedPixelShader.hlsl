struct PixelShaderInput
{
    float4 Position : SV_Position;
    float2 TextCoord : TEXCOORD;
};

Texture2D text : register(t0);
SamplerState samp : register(s0);

float4 main(PixelShaderInput IN) : SV_Target
{
    return text.Sample(samp, IN.TextCoord.xy);
}