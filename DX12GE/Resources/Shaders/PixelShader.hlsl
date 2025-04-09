struct PixelShaderInput
{
    float4 Position : SV_Position;
    float4 Normal : NORMAL;
    float2 TextCoord : TEXCOORD;
    float4 ShadowPos0 : POSITION0;
    float4 ShadowPos1 : POSITION1;
    float4 ShadowPos2 : POSITION2;
    float4 WorldPos : POSITION4;
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

// ВАЖНО: количество флоатов в любом константном буфере должно быть кратно 4-м
cbuffer WCB : register(b2)
{
    AmbientLight AmbientLightCB;
    DirectionalLight DirectionalLightCB;
    LightProperties LightPropertiesCB;
};

StructuredBuffer<PointLight> PointLightsSB : register(t0);
StructuredBuffer<SpotLight> SpotLightsSB : register(t1);
Texture2D DiffuseTextureSB : register(t2);
Texture2D ShadowMapSB0 : register(t3);
Texture2D ShadowMapSB1 : register(t4);
Texture2D ShadowMapSB2 : register(t5);

SamplerState StaticSampler : register(s0);
SamplerState ShadowSampler : register(s1);

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
        
        if (depth + 0.0001 < ProjCoords.z)
        {
            shadow += 0.0;
        }
        else
        {
            shadow += 1.0;
        }
    }
    
    return shadow / 9.0f;
};

float4 CalcLightInternal(float3 Color, float Intensity, float3 pLightDirection, float3 Normal, float3 WorldPos)
{
    float3 LightDirection = normalize(pLightDirection);
    float DiffuseFactor = dot(Normal, -LightDirection);
    
    float4 DiffuseColor = float4(0.0, 0.0, 0.0, 0.0);
    float4 SpecularColor = float4(0.0, 0.0, 0.0, 0.0);
    
    if (DiffuseFactor > 0)
    {
        DiffuseColor = float4(Color, 1.0) * Intensity * DiffuseFactor;
        float3 VertexToEye = normalize(LightPropertiesCB.CameraPos.xyz - WorldPos);
        float3 LightReflect = normalize(reflect(pLightDirection, Normal));
        float SpecularFactor = dot(VertexToEye, LightReflect);
        SpecularFactor = pow(SpecularFactor, LightPropertiesCB.MaterialPower);
        if (SpecularFactor > 0)
        {
            SpecularColor = float4(Color, 1.0) * LightPropertiesCB.Intensity * SpecularFactor;
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


float4 main(PixelShaderInput IN) : SV_Target
{
    float4 texel = DiffuseTextureSB.Sample(StaticSampler, IN.TextCoord);
    
    if (texel.a < 0.3) discard;
    
    float4 AmbientColor = float4(AmbientLightCB.Color, 1.0) * AmbientLightCB.Intensity;
    
    float shadowFactor = 0.0;
    float Distance = length(IN.WorldPos.xyz - LightPropertiesCB.CameraPos.xyz);
    
    //int s = LightPropertiesCB.PointLightsCount + LightPropertiesCB.SpotLightsCount;
    
    if (Distance < 20)
        shadowFactor = CalcShadowFactor(IN.ShadowPos0, ShadowMapSB0);
    else if (Distance < 40)
        shadowFactor = CalcShadowFactor(IN.ShadowPos1, ShadowMapSB1);
    else 
        shadowFactor = CalcShadowFactor(IN.ShadowPos2, ShadowMapSB2);
    
    
    float4 DirectionalColor = 
        CalcLightInternal(
            DirectionalLightCB.Color, 
            DirectionalLightCB.Intensity, 
            normalize(DirectionalLightCB.Direction.xyz),
            IN.Normal.xyz, 
            IN.WorldPos.xyz) 
        * shadowFactor;
    
    float3 ResultLightIntensity = AmbientColor + DirectionalColor;  
    
    for (int i = 0; i < LightPropertiesCB.PointLightsCount; i++)
    {
        ResultLightIntensity += CalcPointLight(PointLightsSB[i], IN.Normal.xyz, IN.WorldPos.xyz);
    }
    
    for (int i = 0; i < LightPropertiesCB.SpotLightsCount; i++)
    {
        ResultLightIntensity += CalcSpotLight(SpotLightsSB[i], IN.Normal.xyz, IN.WorldPos.xyz);
    }
    
    return texel * float4(ResultLightIntensity, 1.0);
    
}