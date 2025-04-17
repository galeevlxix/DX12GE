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

cbuffer WCB : register(b0)
{
    AmbientLight AmbientLightCB;
    DirectionalLight DirectionalLightCB;
    LightProperties LightPropertiesCB;
};

// GBuffer textures
Texture2D<float4> gPosition : register(t0);     // RGBA32_FLOAT
Texture2D<float4> gNormal : register(t1);       // RGBA16_FLOAT
Texture2D<float4> gDiffuse : register(t2);      // RGBA8_UNORM

SamplerState gSampler : register(s0);

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

float4 main(PSInput input) : SV_Target
{
    float2 uv = input.TexCoord;

    float3 worldPos = gPosition.Sample(gSampler, uv).xyz;
    float3 normal = normalize(gNormal.Sample(gSampler, uv).xyz);
    float3 albedo = gDiffuse.Sample(gSampler, uv).rgb;

    // Ambient
    float3 ambient = albedo * AmbientLightCB.Color * AmbientLightCB.Intensity;

    // Diffuse
    float3 diffuse =
        CalcLightInternal(
            DirectionalLightCB.Color,
            DirectionalLightCB.Intensity,
            normalize(DirectionalLightCB.Direction.xyz),
            normal,
            worldPos);

    float3 result = ambient + diffuse;

    return float4(result, 1.0);
}