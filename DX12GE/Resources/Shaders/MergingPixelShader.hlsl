struct PSInput
{
    float4 Position : SV_POSITION;
    float2 TexCoord : TEXCOORD;
};

Texture2D<float4> gColor : register(t0);    // RGBA8_UNORM
Texture2D<float4> gSSR : register(t1);      // RGBA8_UNORM
Texture2D<float4> gORM : register(t2);      // RGBA8_UNORM

SamplerState gSampler : register(s0);

float4 SsrGaussianBlur(float2 uv, int radius)
{
    if (radius == 0) 
        return gSSR.Sample(gSampler, uv);
    
    float2 textureSize;
    gSSR.GetDimensions(textureSize.x, textureSize.y);
    float2 texelSize = 1.0 / textureSize;
    
    float sigma = float(radius) / 2.0;
    float twoSigma2 = 2.0 * sigma * sigma;

    float4 sum = float4(0.0, 0.0, 0.0, 0.0);
    float weightSum = 0.0;

    [loop]
    for (int y = -radius; y <= radius; y++)
    {
        [loop]
        for (int x = -radius; x <= radius; x++)
        {
            float dist2 = float(x * x + y * y);
            float weight = exp(-dist2 / twoSigma2);

            float2 offset = float2(float(x), float(y)) * texelSize;
            float4 color = gSSR.Sample(gSampler, uv + offset);
            sum += color * weight;
            weightSum += weight;
        }
    }

    return sum / weightSum;
}

float4 main(PSInput input) : SV_Target
{
    float4 color = gColor.Sample(gSampler, input.TexCoord); 
    float4 ssr = gSSR.Sample(gSampler, input.TexCoord);    
    
    float3 result = color.rgb + lerp(0.0, ssr.rgb, ssr.a);
    
    result = result / (result + 1.0);
    result = pow(abs(result), 1.0f / 2.2f);
    return float4(result, 1.0);
}