struct PixelShaderInput
{
    float4 Position : SV_Position;
    float4 Normal : NORMAL;
    float2 TextCoord : TEXCOORD;
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
    //return float4(IN.TextCoord.x, 0.0, IN.TextCoord.y, 1.0) * float4(lightIntensity, 1.0);
}