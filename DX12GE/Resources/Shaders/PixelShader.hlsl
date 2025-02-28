struct PixelShaderInput
{
    float4 Color : COLOR;
    float4 Normal : NORMAL;
};

float4 main(PixelShaderInput IN) : SV_Target
{
    float3 ambientLightIntensity = float3(0.3, 0.3, 0.3);
    float3 sunLightIntensity = float3(1, 1, 1);
    float3 sunLightDirection = normalize(float3(-1, 1, 0));
    
    float3 lightIntensity = ambientLightIntensity + sunLightIntensity * max(dot(IN.Normal.xyz, sunLightDirection), 0.0f);

    return IN.Color * float4(lightIntensity, 1.0);
}