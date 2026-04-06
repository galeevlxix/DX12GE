struct PSInput
{
    float4 Position : SV_Position;
    float3 Direction : TEXCOORD0;
};

TextureCube<float4> SkyboxCubemap : register(t0);
SamplerState StaticSampler : register(s0);

float4 main(PSInput IN) : SV_TARGET
{
    return SkyboxCubemap.Sample(StaticSampler, IN.Direction);
}