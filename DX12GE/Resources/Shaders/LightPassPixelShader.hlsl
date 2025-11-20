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
};

struct SpotLight
{
    float3 Color;
    float Intensity;
    float3 Position;
    float AttenuationConstant;
    float AttenuationLinear;
    float AttenuationExp;
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
Texture2D<float4> gORM      : register(t10);

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
static float fogDistance = 115 * 2;         //fogEnd - fogStart

static float PI = 3.14159265359;

float CalcShadowFactor(float4 ShadowPos, Texture2D ShadowMapSB)
{
    // Complete projection by doing division by w.
    float3 ProjCoords = ShadowPos.xyz / ShadowPos.w;
    ProjCoords.y = -ProjCoords.y;
    ProjCoords.xy = 0.5 * ProjCoords.xy + 0.5;
    
    uint width, height;
    ShadowMapSB.GetDimensions(width, height);

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

float3 DebugShadowCascade(float3 WorldPos, float Distance)
{
    if (Distance < splitDistances[0])
        return float3(1.5f, 1, 1);
    else if (Distance < splitDistances[1])
        return float3(1, 1.5f, 1);
    else if (Distance < splitDistances[2])
        return float3(1, 1, 1.5f);
    else
        return float3(1, 1.5f, 1.5f);
}

float DistributionGGX(float3 N, float3 H, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;
    
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom + 1e-5;
    
    return a2 / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;
    
    return NdotV / (NdotV * (1.0 - k) + k);
}

float GeometrySmith(float3 N, float3 V, float3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    return GeometrySchlickGGX(NdotV, roughness) * GeometrySchlickGGX(NdotL, roughness);
}

float3 FresnelSchlick(float cosTheta, float3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

float3 CalculatePBRLight(float3 Color, float Intensity, float3 LightDirection, float3 worldPos, float3 normal, float4 albedo, float roughness, float metalness)
{
    float3 PixelToEye = normalize(LightPropertiesCB.CameraPos.xyz - worldPos);
    
    float3 H = normalize(PixelToEye + LightDirection);
    float3 F0 = lerp(0.04, albedo.rgb, metalness);
    
    float NDF = DistributionGGX(normal, H, roughness);
    float G = GeometrySmith(normal, PixelToEye, LightDirection, roughness);
    float3 F = FresnelSchlick(max(dot(H, PixelToEye), 0.0), F0);
    
    float3 numerator = NDF * G * F;
    float denominator = 4.0 * max(dot(normal, PixelToEye), 0.0) * max(dot(normal, LightDirection), 0.0);
    float3 specular = numerator / max(denominator, 0.001);
    
    float3 kS = F;
    float3 kD = (1.0 - kS) * (1.0 - metalness);
    
    float NdotL = max(dot(normal, LightDirection), 0.0);
    return (kD * albedo.rgb / PI + specular) * Color * Intensity * NdotL;
}

float3 CalcPointLight(PointLight pLight, float3 worldPos, float3 normal, float4 albedo, float roughness, float metalness)
{
    float3 LightToPixel = worldPos - pLight.Position;
    float Distance = length(LightToPixel);
    float3 Color = CalculatePBRLight(pLight.Color, pLight.Intensity, -normalize(LightToPixel), worldPos, normal, albedo, roughness, metalness);
    float Attenuation = pLight.AttenuationConstant + pLight.AttenuationLinear * Distance + pLight.AttenuationExp * Distance * Distance;
    return Color / Attenuation;
}

float3 CalcSpotLight(SpotLight sLight, float3 worldPos, float3 normal, float4 albedo, float roughness, float metalness)
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
        
        float3 Color = CalcPointLight(pointLight, worldPos, normal, albedo, roughness, metalness);
        return Color * (1.0 - (1.0 - SpotFactor) * 1.0 / (1.0 - sLight.Cutoff));
    }

    return float3(0, 0, 0);
}

float4 main(PSInput input) : SV_Target
{
    float3 worldPos = gPosition.Sample(gSampler, input.TexCoord).xyz;
    float3 normal = normalize(gNormal.Sample(gSampler, input.TexCoord).xyz);
    float4 albedo = abs(gDiffuse.Sample(gSampler, input.TexCoord));
    float4 emissive = gEmissive.Sample(gSampler, input.TexCoord);
    float4 orm = gORM.Sample(gSampler, input.TexCoord);
    float roughness = orm.g;
    float metalness = orm.b;
    
    if (albedo.a == 0.0)
        discard;
    albedo = pow(albedo, 2.2f);
    
    float3 cameraPixelVector = worldPos.xyz - LightPropertiesCB.CameraPos.xyz;
    float3 cameraPixelDirection = normalize(cameraPixelVector);
    float cameraPixelDistance = length(cameraPixelVector);
    
    // Ambient
    float3 ambient = albedo.rgb * AmbientLightCB.Color * AmbientLightCB.Intensity;

    // Diffuse
    float3 diffuse =
        CalculatePBRLight(
            DirectionalLightCB.Color,
            DirectionalLightCB.Intensity,
            -normalize(DirectionalLightCB.Direction.xyz),
            worldPos, normal, albedo, roughness, metalness);
    
    float shadowFactor = CalcShadowCascade(worldPos, cameraPixelDistance);
    float3 lightingResult = ambient + diffuse * shadowFactor;// * DebugShadowCascade(worldPos, cameraPixelDistance).xyz;
    
    // Pointlights
    for (int pIndex = 0; pIndex < LightPropertiesCB.PointLightsCount; pIndex++)
    {
        lightingResult += CalcPointLight(PointLightsSB[pIndex], worldPos, normal, albedo, roughness, metalness);
    }
    
    // Spotlights
    for (int sIndex = 0; sIndex < LightPropertiesCB.SpotLightsCount; sIndex++)
    {
        lightingResult += CalcSpotLight(SpotLightsSB[sIndex], worldPos, normal, albedo, roughness, metalness);
    }
    
    float3 outputPixelColor = lightingResult + emissive.rgb;
    
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