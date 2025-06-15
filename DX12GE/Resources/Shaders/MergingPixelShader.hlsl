struct PSInput
{
    float4 Position : SV_POSITION;
    float2 TexCoord : TEXCOORD;
};

Texture2D<float4> gColor : register(t0);    // RGBA8_UNORM
Texture2D<float4> gSSR : register(t1);      // RGBA8_UNORM

SamplerState gSampler : register(s0);

float4 main(PSInput input) : SV_Target
{
    float4 color = gColor.Sample(gSampler, input.TexCoord);
    float4 ssr = gSSR.Sample(gSampler, input.TexCoord);
    float3 result = lerp(color.rgb, ssr.rgb, ssr.a);
    return float4(result, 1.0);
}