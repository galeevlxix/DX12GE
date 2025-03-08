struct PixelShaderInput
{
    float4 Position : SV_Position;
    float4 Normal : NORMAL;
    float2 TextCoord : TEXCOORD;
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
    
    float3 Direction;
};

struct PointLight
{
    float3 Color;
    float Intensity;
    
    float AttenuationConstant;
    float AttenuationLinear;
    float AttenuationExp;
};

struct SpotLight
{
    float3 Color;
    float Intensity;
    
    float3 Direction;
    
    float AttenuationConstant;
    float AttenuationLinear;
    float AttenuationExp;
};


Texture2D text : register(t0);
SamplerState samp : register(s0);


float4 main(PixelShaderInput IN) : SV_Target
{
    float4 texel = text.Sample(samp, IN.TextCoord);
    if (texel.a < 0.3)
        discard;
    float3 ambientLightIntensity = float3(0.3, 0.3, 0.3);
    float3 sunLightIntensity = float3(1, 1, 1);
    float3 sunLightDirection = normalize(float3(-1, 1, -1));
    
    float3 lightIntensity = ambientLightIntensity + sunLightIntensity * max(dot(normalize(IN.Normal.xyz), sunLightDirection), 0.0f);

    return texel * float4(lightIntensity, 1.0);
}