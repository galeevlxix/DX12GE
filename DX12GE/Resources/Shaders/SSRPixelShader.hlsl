struct PSInput
{
    float4 Position : SV_POSITION;
    float2 TexCoord : TEXCOORD;
};

cbuffer SSRCB : register(b0)
{
    matrix ViewProjection;
    float4 CameraPos;
    float RayStep;
    //int MaxSteps;
    float MaxDistance;
    float Thickness;
};

Texture2D<float4> gPosition : register(t0);
Texture2D<float4> gNormal   : register(t1);
Texture2D<float4> gORM      : register(t2);
Texture2D<float4> gColor    : register(t3);

SamplerState gSampler : register(s0);

static const float MinCamDist = 8.0f;
static const float MaxCamDist = 32.0f;
static const float CamDistDiff = MaxCamDist - MinCamDist;

static float f0 = 0.04;

float3 TraceScreenSpaceReflection(float3 worldPos, float3 reflectionDir, float stepLength, float thickness)
{
    float3 rayStep = reflectionDir * stepLength;
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
            return float3(0, 0, 0);
        
        float3 gBufferWorldPos = gPosition.SampleLevel(gSampler, uv, 0).xyz;
        float depthDiff = length(currentPos - gBufferWorldPos);
        
        if (depthDiff > 0 && depthDiff <= thickness)
        {
            float3 normal = gNormal.Sample(gSampler, uv).xyz;
            if (dot(reflectionDir, normal) > 0)
                return float3(0, 0, 0);
            
            return gColor.Sample(gSampler, uv).rgb;
        }
    }
    
    return float3(0, 0, 0);
}

float4 main(PSInput input) : SV_Target
{
    float4 orm = gORM.Sample(gSampler, input.TexCoord);
    
    if (orm.b < 0.025)
        return float4(0.0, 0.0, 0.0, 0.0);
    
    float3 worldPos = gPosition.Sample(gSampler, input.TexCoord).xyz;
    float3 normal = normalize(gNormal.Sample(gSampler, input.TexCoord).xyz);
    
    float3 cameraPixelVector = worldPos - CameraPos.xyz;
    
    float fresnel = saturate(f0 + (1 - f0) * pow(1 - dot(normal, normalize(-cameraPixelVector)), 5));
    /*if (fresnel < 0.5f)
        return float4(0.0, 0.0, 0.0, fresnel);*/
    
    float camDist = length(cameraPixelVector);
    float camDistFactor = saturate((camDist - MinCamDist) / CamDistDiff);
    
    float stepLength = lerp(0.025, 0.3, camDistFactor);
    float thickness = lerp(0.02, 0.24, camDistFactor);
    
    float3 cameraPixelDirection = normalize(cameraPixelVector.xyz);    
    float3 reflectDir = normalize(reflect(cameraPixelDirection, normal));
    
    float3 reflectionColor = TraceScreenSpaceReflection(worldPos, reflectDir, stepLength, thickness);
    
    
    return float4(reflectionColor, fresnel);
}
