struct PixelShaderInput
{
    float4 Position : SV_Position;
    float4 Normal : NORMAL;
    float2 TextCoord : TEXCOORD;
    float4 ShadowPos : POSITION;
};

Texture2D text : register(t0);
SamplerState samp : register(s0);

struct AmbientLight
{
    float3 Color;
    float Intensity;
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
    
    float AttenuationConstant;
    float AttenuationLinear;
    float AttenuationExp;
    
    float3 Direction;
    
    float Cutoff;
};

struct SpecularLight
{
    float3 CameraPos;
    float Intensity;
    float MaterialPower;
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
//StructuredBuffer<SpotLight> SpotLightsCB : register(t2);

//ConstantBuffer<SpecularLight> SpecularLightCB : register(b3);

Texture2D shadowMap : register(t1);
SamplerState shadowSampler : register(s1);

//ConstantBuffer<DirLightShadowMapTransform> ShadowMapTransformCB : register(b4);

float CalcShadowFactor(float4 ShadowPos)
{
    // Complete projection by doing division by w.
    float3 ProjCoords = ShadowPos.xyz / ShadowPos.w;
    ProjCoords.y = -ProjCoords.y;    
    ProjCoords.xy = 0.5 * ProjCoords.xy + 0.5;
    
    /*
    float depth = shadowMap.Sample(shadowSampler, ProjCoords.xy).r;
    
    if (depth + 0.0001 < ProjCoords.z)
    {
        return 0.0;
    }
    else
    {
        return 1.0;
    }
    */
    
    // Depth in NDC space.
    //float depth = ProjCoords.z;

    uint width, height, numMips;
    shadowMap.GetDimensions(0, width, height, numMips);

    // Texel size.
    float dx = 1.0f / (float) width;

    const float2 offsets[9] =
    {
        float2(-dx, -dx), float2(0.0f, -dx), float2(dx, -dx),
        float2(-dx, 0.0f), float2(0.0f, 0.0f), float2(dx, 0.0f),
        float2(-dx, +dx), float2(0.0f, +dx), float2(dx, +dx)
    };
    
    float shadow = 0.0;

    [unroll]
    for (int i = 0; i < 9; ++i)
    {
        float depth = shadowMap.Sample(shadowSampler, ProjCoords.xy + offsets[i]).r;
        
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

float4 CalcLightInternal(float3 Color, float Intensity, float3 pLightDirection, float3 Normal)
{
    float3 LightDirection = normalize(pLightDirection);
    float DiffuseFactor = dot(Normal, -LightDirection);
    
    float4 DiffuseColor = float4(0.0, 0.0, 0.0, 0.0);
    float4 SpecularColor = float4(0.0, 0.0, 0.0, 0.0);
    
    if (DiffuseFactor > 0)
    {
        DiffuseColor = float4(Color, 1.0) * Intensity * DiffuseFactor;
    }
    
    return DiffuseColor;
}


float4 main(PixelShaderInput IN) : SV_Target
{
    float4 texel = text.Sample(samp, IN.TextCoord);
    
    if (texel.a < 0.3)
        discard;
    
    float4 AmbientColor = float4(AmbientLightCB.Color, 1.0) * AmbientLightCB.Intensity;
    float4 DirectionalColor = CalcLightInternal(DirectionalLightCB.Color, DirectionalLightCB.Intensity, DirectionalLightCB.Direction, normalize(IN.Normal).xyz);
    float3 ResultLightIntensity = AmbientColor + 
    CalcShadowFactor(IN.ShadowPos) * DirectionalColor;  
    
    return texel * float4(ResultLightIntensity, 1.0);
}