struct PSInput
{
    float4 Position : SV_POSITION;
    float2 TexCoord : TEXCOORD;
};

Texture2D<float4> gFinalResult : register(t0); // RGBA8_UNORM

SamplerState gSampler : register(s0);

float4 main(PSInput input) : SV_Target
{
    return gFinalResult.Sample(gSampler, input.TexCoord);
}