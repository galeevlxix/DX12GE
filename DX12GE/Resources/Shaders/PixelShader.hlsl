struct PixelShaderInput
{
    float4 Position : SV_Position;
    float4 Normal : NORMAL;
    float2 TextCoord : TEXCOORD;
};

float4 main(PixelShaderInput IN) : SV_Target
{
    float3 ambientLightIntensity = float3(0.3, 0.3, 0.3);
    float3 sunLightIntensity = float3(1, 1, 1);
    float3 sunLightDirection = normalize(float3(-1, 1, 0));
    
    float3 lightIntensity = ambientLightIntensity + sunLightIntensity * max(dot(IN.Normal.xyz, sunLightDirection), 0.0f);

    return float4(IN.TextCoord.x, IN.TextCoord.y, 0.0, 1.0) * float4(lightIntensity, 1.0);
}