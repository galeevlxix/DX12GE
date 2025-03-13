struct PixelShaderInput
{
    float4 Position : SV_Position;
    float4 Normal : NORMAL;
    float2 TextCoord : TEXCOORD;
    float3 WorldPos : POSITION;
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

//ConstantBuffer<LightProperties> LightPropertiesCB : register(b2);

//StructuredBuffer<PointLight> PointLightsCB : register(t1);
//ConstantBuffer<SpotLight> SpotLightsCB : register(t4);

float4 CalcLightInternal(float3 Color, float Intensity, float3 pLightDirection, float3 Normal)
{
    float3 LightDirection = normalize(pLightDirection);
    float DiffuseFactor = dot(Normal, -LightDirection);
    return DiffuseFactor > 0 ? float4(Color, 1.0) * Intensity * DiffuseFactor : float4(0, 0, 0, 0);
}

float4 CalcPointLight(PointLight pLight, float3 normal, float3 worldPos)
{
    float3 LightDirection = worldPos - pLight.Position;
    float Distance = length(LightDirection);
    float4 Color = CalcLightInternal(pLight.Color, pLight.Intensity, LightDirection, normal);
    float Attenuation = pLight.AttenuationConstant + pLight.AttenuationLinear * Distance + pLight.AttenuationExp * Distance * Distance;
    return Color / Attenuation;
}

float4 main(PixelShaderInput IN) : SV_Target
{
    float4 texel = text.Sample(samp, IN.TextCoord);
    if (texel.a < 0.3)
        discard;
    
    float4 AmbientColor = float4(AmbientLightCB.Color, 1.0) * AmbientLightCB.Intensity;
    float4 DirectionalColor = CalcLightInternal(DirectionalLightCB.Color, DirectionalLightCB.Intensity, DirectionalLightCB.Direction, normalize(IN.Normal).xyz);
    float3 ResultLightIntensity = AmbientColor + DirectionalColor;
    
    //for (int i = 0; i < LightPropertiesCB.PointLightsCount; i++)
    //{
    //    ResultLightIntensity += CalcPointLight(PointLightsCB[i], IN.Normal.xyz, IN.WorldPos);
    //}

    return texel * float4(ResultLightIntensity, 1.0);
}