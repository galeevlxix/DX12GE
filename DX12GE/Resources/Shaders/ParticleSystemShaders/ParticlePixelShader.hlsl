struct PSInput
{
    float4 Position : SV_Position;
    float4 Normal : NORMAL;
    float2 TextCoord : TEXCOORD;
    float4 WorldPos : POSITION;
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

cbuffer WCB : register(b1)
{
    AmbientLight AmbientLightCB;
    DirectionalLight DirectionalLightCB;
    LightProperties LightPropertiesCB;
};

Texture2D DiffuseTextureSB : register(t0);
SamplerState StaticSampler : register(s0);

static bool fogEnable = true;
static float fogStart = 35;
static float fogDistance = 115; //fogEnd - fogStart

float4 CalcLightInternal(float3 Color, float Intensity, float3 pLightDirection, float3 Normal, float3 WorldPos)
{
    float DiffuseFactor = dot(Normal, -pLightDirection);
    
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

float4 main(PSInput IN) : SV_Target
{
    float4 pixelColor = DiffuseTextureSB.Sample(StaticSampler, IN.TextCoord);
    
    if (pixelColor.a < 0.01)
        discard;
    
    float averAlpha = (pixelColor.r + pixelColor.g + pixelColor.b) / 3.0;
    if (averAlpha < 0.01)
        discard;    
    pixelColor.a = averAlpha;
    
    // Fog
    if (fogEnable)
    {
        float3 color = pixelColor.rgb;
        float cameraPixelDistance = length(IN.WorldPos.xyz - LightPropertiesCB.CameraPos.xyz);
        float fogFactor = 1.0f - (cameraPixelDistance - fogStart) / fogDistance;
        fogFactor = clamp(fogFactor, 0.0f, 1.0f);
        float3 fogColor = float3(0.5f, 0.5f, 0.5f);
        color = fogFactor * color + (1.0 - fogFactor) * fogColor;
        pixelColor = float4(color, pixelColor.a);
    }
    
    return pixelColor;
}