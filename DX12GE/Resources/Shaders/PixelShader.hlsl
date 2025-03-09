struct PixelShaderInput
{
    float4 Position : SV_Position;
    float4 Normal : NORMAL;
    float2 TextCoord : TEXCOORD;
};

Texture2D text : register(t0);
SamplerState samp : register(s0);

struct AmbientLight
{
    float3 Color;
    float Intensity;
    //16 bytes
};

struct DirectionalLight
{
    float3 Color;
    float Intensity;
    
    float3 Direction;
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
    float3 Direction;
    
    float AttenuationConstant;
    float AttenuationLinear;
    float AttenuationExp;
};

struct LightProperties
{
    uint PointLightsCount;
    uint SpotLightsCount;
};

ConstantBuffer<AmbientLight> AmbientLightCB : register(b0, space1);
ConstantBuffer<DirectionalLight> DirectionalLightCB : register(b1);
//StructuredBuffer<PointLight> PointLightsCB : register(t3);
//StructuredBuffer<SpotLight> SpotLightsCB : register(t4);

//ConstantBuffer<LightProperties> LightPropertiesCB : register(b0, space1);

float4 CalcLightInternal(float3 Color, float Intensity, float3 pLightDirection, float3 Normal)
{
    float3 LightDirection = normalize(pLightDirection);
    float DiffuseFactor = dot(Normal, -LightDirection);
    return DiffuseFactor > 0 ? float4(Color, 1.0) * Intensity * DiffuseFactor : float4(0, 0, 0, 0);
}


float4 main(PixelShaderInput IN) : SV_Target
{
    float4 texel = text.Sample(samp, IN.TextCoord);
    if (texel.a < 0.3)
        discard;
    
    float4 AmbientColor = float4(AmbientLightCB.Color, 1.0) * AmbientLightCB.Intensity;
    float4 DirectionalColor = CalcLightInternal(DirectionalLightCB.Color, DirectionalLightCB.Intensity, DirectionalLightCB.Direction, normalize(IN.Normal).xyz);
    float3 lightIntensity = AmbientColor + DirectionalColor;

    return texel * float4(lightIntensity, 1.0);
}