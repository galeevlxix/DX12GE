struct PSInput
{
    float4 Position : SV_POSITION;
    float2 TexCoord : TEXCOORD;
};

cbuffer SSRCB : register(b0)
{
    matrix ViewProjection;
    float4 CameraPos;
    float RayStepLength;
    float MaxDistance;
    float Thickness;
};

Texture2D<float4> gPosition : register(t0);
Texture2D<float4> gNormal   : register(t1);
Texture2D<float4> gORM      : register(t2);
Texture2D<float4> gColor    : register(t3);
TextureCube<float4> SkyboxCubemap : register(t4);

SamplerState gSampler : register(s0);

float3 TraceScreenSpaceReflection(float3 worldPos, float3 reflectionDir)
{    
    float3 rayStep = reflectionDir * RayStepLength;
    float3 currentPos = worldPos;
    
    [loop]
    while (length(worldPos - currentPos) < MaxDistance)
    {
        currentPos += rayStep;
        
        float4 clipPos = mul(ViewProjection, float4(currentPos, 1.0));
        clipPos.xyz /= clipPos.w;
        clipPos.y = -clipPos.y;
        float2 uv = clipPos.xy * 0.5 + 0.5;
        
        if (uv.x < 0 || uv.y < 0 || uv.x > 1 || uv.y > 1)
            return SkyboxCubemap.Sample(gSampler, reflectionDir).rgb;
        
        float3 gBufferWorldPos = gPosition.SampleLevel(gSampler, uv, 0).xyz;
        float depthDiff = length(currentPos - gBufferWorldPos);
        
        if (depthDiff > 0 && depthDiff <= Thickness)
        {
            float3 normal = gNormal.Sample(gSampler, uv).xyz;
            if (dot(reflectionDir, normal) > 0)
                return SkyboxCubemap.Sample(gSampler, reflectionDir).rgb;
            
            float3 color = gColor.Sample(gSampler, uv).rgb;
            return color;
        }
    }
    
    return SkyboxCubemap.Sample(gSampler, reflectionDir).rgb;
}

float4 main(PSInput input) : SV_Target
{
    float4 orm = gORM.Sample(gSampler, input.TexCoord);
    if (orm.b < 0.16)
        return float4(0.0, 0.0, 0.0, 0.0);
    
    float3 normal = normalize(gNormal.Sample(gSampler, input.TexCoord).xyz);
    float3 worldPos = gPosition.Sample(gSampler, input.TexCoord).xyz + normal * 0.001;
    float3 cameraPixelVector = normalize(worldPos - CameraPos.xyz);
    
    float f0 = 0.04;
    float NdotV = dot(normal, -cameraPixelVector);
    float fresnel = saturate(f0 + (1 - f0) * pow(1 - NdotV, 5));
   
    float3 reflectDir = normalize(reflect(cameraPixelVector, normal));
    
    float3 reflectionColor = TraceScreenSpaceReflection(worldPos, reflectDir);
    
    return float4(reflectionColor, fresnel);
}
