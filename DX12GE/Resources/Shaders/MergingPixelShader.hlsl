struct PSInput
{
    float4 Position : SV_POSITION;
    float2 TexCoord : TEXCOORD;
};

Texture2D<float4> gColor : register(t0);    // RGBA8_UNORM
Texture2D<float4> gSSR : register(t1);      // RGBA8_UNORM
Texture2D<float4> gORM : register(t2);      // RGBA8_UNORM

SamplerState gSampler : register(s0);

float3 AcesToneMap(float3 color)
{
    float a = 2.51;
    float b = 0.03;
    float c = 2.43;
    float d = 0.59;
    float e = 0.14;
    return clamp((color * (a * color + b)) / (color * (c * color + d) + e), 0.0, 1.0);
}

float3 linearToSRGB(float3 color)
{
    float3 sRGB;
    for (int i = 0; i < 3; i++)
    {
        sRGB[i] = color[i] <= 0.0031308 ? color[i] * 12.92 : 1.055 * pow(color[i], 1.0 / 2.4) - 0.055;
    }
    return sRGB;
}

float4 main(PSInput input) : SV_Target
{
    float4 color = gColor.Sample(gSampler, input.TexCoord); 
    float4 ssr = gSSR.Sample(gSampler, input.TexCoord);    
    
    float3 result = color.rgb + lerp(0.0, ssr.rgb, ssr.a);
    
    result = AcesToneMap(result);
    result = linearToSRGB(result);
    return float4(result, 1.0);
}