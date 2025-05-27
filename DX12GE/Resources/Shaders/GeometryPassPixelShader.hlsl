struct PSInput
{
    float4 Position :   SV_Position;
    float4 Normal :     NORMAL;
    float2 TextCoord :  TEXCOORD;
    float4 WorldPos :   POSITION0;
    float4 Tangent : TANGENT;
    float4 Bitangent : BITANGENT;
};

struct PSOutput
{
    float4 Position :   SV_Target0;
    float4 Normal :     SV_Target1;
    float4 Diffuse :    SV_Target2;
};

cbuffer MCB : register(b1)
{
    float4 HasDiffuseNormalEmissive;
    float4 HasMetallicRoughnessOcclusion;
};

Texture2D DiffuseTextureSB : register(t0);
Texture2D NormalTextureSB : register(t1);
SamplerState StaticSampler : register(s0);

float4 CalculateNormalMap(float3 Normal0, float3 Tangent0, float3 Bitangent0, float2 texCoord0)
{
    float3 normalMap = NormalTextureSB.Sample(StaticSampler, texCoord0).rgb;
    normalMap = normalMap * 2.0f - float3(1.0f, 1.0f, 1.0f);
    
    float3x3 TBN = float3x3(normalize(Tangent0), normalize(Bitangent0), normalize(Normal0));
    float3 newNorm = normalize(mul(normalMap, TBN));
    return float4(newNorm, 0.0);
}

PSOutput main(PSInput IN)
{
    PSOutput OUT;
    
    OUT.Position = IN.WorldPos;
    OUT.Normal = 
        HasDiffuseNormalEmissive.y > 0.5f ? 
        CalculateNormalMap(IN.Normal.rgb, IN.Tangent.rgb, IN.Bitangent.rgb, IN.TextCoord) : 
        IN.Normal;
    OUT.Diffuse = DiffuseTextureSB.Sample(StaticSampler, IN.TextCoord);
    
    return OUT;
}