struct PSInput
{
    float4 Position : SV_POSITION;
    float2 TexCoord : TEXCOORD;
};

struct AmbientLight
{
    float3 Color;
    float Intensity;
};

struct DirectionalLight
{
    float3 Color;
    float Intensity;
    float4 Direction;
};

struct LightProperties
{
    float4 CameraPos;
    float Intensity;
    float MaterialPower;
    float PointLightsCount;
    float SpotLightsCount;
};

struct PointLight
{
    float3 Color;
    float Intensity;
    float3 Position;
    float AttenuationConstant;
    float AttenuationLinear;
    float AttenuationExp;
    float MaxRadius;
};

struct SpotLight
{
    float3 Color;
    float Intensity;
    float3 Position;
    float AttenuationConstant;
    float AttenuationLinear;
    float AttenuationExp;
    float MaxRadius;
    float3 Direction;
    float Cutoff;
};

cbuffer WCB : register(b0)
{
    AmbientLight AmbientLightCB;
    DirectionalLight DirectionalLightCB;
    LightProperties LightPropertiesCB;
};

cbuffer SCB : register(b1)
{
    matrix ShadowMapTransform0;
    matrix ShadowMapTransform1;
    matrix ShadowMapTransform2;
    matrix ShadowMapTransform3;
};

// GBuffer textures
Texture2D<float4> gPosition : register(t0);
Texture2D<float4> gNormal   : register(t1);  
Texture2D<float4> gDiffuse  : register(t2); 
Texture2D<float4> gEmissive : register(t9);

StructuredBuffer<PointLight> PointLightsSB : register(t3);
StructuredBuffer<SpotLight> SpotLightsSB : register(t4);

Texture2D ShadowMapSB0 : register(t5);
Texture2D ShadowMapSB1 : register(t6);
Texture2D ShadowMapSB2 : register(t7);
Texture2D ShadowMapSB3 : register(t8);

SamplerState gSampler : register(s0);
SamplerState ShadowSampler : register(s1);

// 0.15f, 0.3f, 0.6f, 1.0f
static float splitDistances[3] = { 37.5, 75, 140 };

static bool fogEnable = false;
static float fogStart = 35;
static float fogDistance = 115;         //fogEnd - fogStart

float CalcShadowFactor(float4 ShadowPos, Texture2D ShadowMapSB)
{
    // Complete projection by doing division by w.
    float3 ProjCoords = ShadowPos.xyz / ShadowPos.w;
    ProjCoords.y = -ProjCoords.y;
    ProjCoords.xy = 0.5 * ProjCoords.xy + 0.5;
    
    uint width, height, numMips;
    ShadowMapSB.GetDimensions(0, width, height, numMips);

    // Texel size.
    float dx = 1.0f / (float) width;

    const float2 offsets[9] =
    {
        float2(-dx, -dx),   float2(0.0f, -dx),  float2(dx, -dx),
        float2(-dx, 0.0f),  float2(0.0f, 0.0f), float2(dx, 0.0f),
        float2(-dx, +dx),   float2(0.0f, +dx),  float2(dx, +dx)
    };
    
    float shadow = 0.0;
    [unroll]
    for (int i = 0; i < 9; ++i)
    {
        float depth = ShadowMapSB.Sample(ShadowSampler, ProjCoords.xy + offsets[i]).r;
        shadow += (depth + 0.0001 < ProjCoords.z) ? 0.0 : 1.0;
    }
    
    return shadow / 9.0f;
};

float CalcShadowCascade(float3 worldPos, float Distance)
{
    if (Distance < splitDistances[0])
        return CalcShadowFactor(mul(ShadowMapTransform0, float4(worldPos, 1.0)), ShadowMapSB0);
    else if (Distance < splitDistances[1])
        return CalcShadowFactor(mul(ShadowMapTransform1, float4(worldPos, 1.0)), ShadowMapSB1);
    else if (Distance < splitDistances[2])
        return CalcShadowFactor(mul(ShadowMapTransform2, float4(worldPos, 1.0)), ShadowMapSB2);
    else
        return CalcShadowFactor(mul(ShadowMapTransform3, float4(worldPos, 1.0)), ShadowMapSB3);
}

float4 DebugShadowCascade(float3 WorldPos, float Distance)
{
    if (Distance < splitDistances[0])
        return float4(1.5f, 1, 1, 1);
    else if (Distance < splitDistances[1])
        return float4(1, 1.5f, 1, 1);
    else if (Distance < splitDistances[2])
        return float4(1, 1, 1.5f, 1);
    else
        return float4(1, 1.5f, 1.5f, 1);
}

float4 CalcLightInternal(float3 Color, float Intensity, float3 pLightDirection, float3 Normal, float3 WorldPos)
{
    float3 LightDirection = normalize(pLightDirection);
    float DiffuseFactor = dot(Normal, -LightDirection);
    
    float4 DiffuseColor = float4(0.0, 0.0, 0.0, 0.0);
    float4 SpecularColor = float4(0.0, 0.0, 0.0, 0.0);
    
    if (DiffuseFactor > 0)
    {
        DiffuseColor = float4(normalize(Color), 1.0) * Intensity * DiffuseFactor;
        float3 VertexToEye = normalize(LightPropertiesCB.CameraPos.xyz - WorldPos);
        float3 LightReflect = normalize(reflect(pLightDirection, Normal));
        float SpecularFactor = dot(VertexToEye, LightReflect);
        SpecularFactor = pow(SpecularFactor, LightPropertiesCB.MaterialPower);
        if (SpecularFactor > 0)
        {
            SpecularColor = float4(Color, 1.0) * Intensity * SpecularFactor;
        }
    }
    
    return DiffuseColor + SpecularColor;
}

float4 CalcPointLight(PointLight pLight, float3 normal, float3 worldPos)
{
    float3 LightDirection = worldPos - pLight.Position;
    float Distance = length(LightDirection);
    float4 Color = CalcLightInternal(pLight.Color, pLight.Intensity, LightDirection, normal, worldPos);
    float Attenuation = pLight.AttenuationConstant + pLight.AttenuationLinear * Distance + pLight.AttenuationExp * Distance * Distance;
    return Color / Attenuation;
}

float4 CalcSpotLight(SpotLight sLight, float3 normal, float3 worldPos)
{
    float3 LightToPixel = normalize(worldPos - sLight.Position);
    float SpotFactor = dot(LightToPixel, sLight.Direction);
    
    if (SpotFactor > sLight.Cutoff)
    {
        PointLight pointLight;
        pointLight.Position = sLight.Position;
        pointLight.Color = sLight.Color;
        pointLight.Intensity = sLight.Intensity;
        pointLight.AttenuationConstant = sLight.AttenuationConstant;
        pointLight.AttenuationLinear = sLight.AttenuationLinear;
        pointLight.AttenuationExp = sLight.AttenuationExp;
        
        float4 Color = CalcPointLight(pointLight, normal, worldPos);
        return Color * (1.0 - (1.0 - SpotFactor) * 1.0 / (1.0 - sLight.Cutoff));
    }

    return float4(0, 0, 0, 0);
}

bool More(float2 v1, float2 v2)
{
    return v1.x > v2.x && v1.y > v2.y;
}

float4 main(PSInput input) : SV_Target
{
    float3 worldPos = gPosition.Sample(gSampler, input.TexCoord).xyz;
    float3 normal = normalize(gNormal.Sample(gSampler, input.TexCoord).xyz);
    float4 albedo = gDiffuse.Sample(gSampler, input.TexCoord);
    float4 emissive = gEmissive.Sample(gSampler, input.TexCoord);
    
    if (albedo.a == 0)
        discard;
    albedo = pow(albedo, 2.2f);
    
    float3 cameraPixelVector = worldPos.xyz - LightPropertiesCB.CameraPos.xyz;
    float3 cameraPixelDirection = normalize(cameraPixelVector);
    float cameraPixelDistance = length(cameraPixelVector);
    
    // Ambient
    float3 ambient = AmbientLightCB.Color * AmbientLightCB.Intensity;

    // Diffuse
    float3 diffuse =
        CalcLightInternal(
            DirectionalLightCB.Color,
            DirectionalLightCB.Intensity,
            DirectionalLightCB.Direction.xyz,
            normal,
            worldPos);
    
    float shadowFactor = CalcShadowCascade(worldPos, cameraPixelDistance);
    float3 lightingResult = ambient + diffuse * shadowFactor; // * DebugShadowCascade(worldPos).xyz;
    
    // Pointlights
    for (int i = 0; i < LightPropertiesCB.PointLightsCount; i++)
    {
        if (length(worldPos - PointLightsSB[i].Position) < PointLightsSB[i].MaxRadius)
        {
            lightingResult += CalcPointLight(PointLightsSB[i], normal, worldPos);
        }
    }
    
    // Spotlights
    for (int i = 0; i < LightPropertiesCB.SpotLightsCount; i++)
    {
        if (length(worldPos - SpotLightsSB[i].Position) < SpotLightsSB[i].MaxRadius)
        {
            lightingResult += CalcSpotLight(SpotLightsSB[i], normal, worldPos);
        }
    }
    
    float3 outputPixelColor = albedo.xyz * lightingResult + emissive.rgb;
    
    outputPixelColor = pow(outputPixelColor, 1.0f / 2.2f);
    
    // Fog
    if (fogEnable)
    {
        float fogFactor = 1.0f - (cameraPixelDistance - fogStart) / fogDistance;    
        fogFactor = clamp(fogFactor, 0.0f, 1.0f);
        float3 fogColor = float3(0.5f, 0.5f, 0.5f);
        outputPixelColor = fogFactor * outputPixelColor + (1.0 - fogFactor) * fogColor;
    }
    
    // Result
    return float4(outputPixelColor, 1.0);
}