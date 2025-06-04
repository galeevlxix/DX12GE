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
    float4 Emissive :   SV_Target3;
    float4 ORM :        SV_Target4;
};

cbuffer MCB : register(b1)
{
    float4 HasDiffuseNormalEmissive;
    float4 HasOcclusionRoughnessMetallicCombined;
};

Texture2D DiffuseTextureSB :    register(t0);
Texture2D NormalTextureSB :     register(t1);
Texture2D EmissiveTextureSB :   register(t2);

Texture2D OcclusionTextureSB :  register(t3);
Texture2D RoughnessTextureSB :  register(t4);
Texture2D MetallicTextureSB :   register(t5);
Texture2D CombinedTextureSB :   register(t6);

SamplerState StaticSampler :    register(s0);

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
    OUT.Diffuse = HasDiffuseNormalEmissive.x > 0.5f ? DiffuseTextureSB.Sample(StaticSampler, IN.TextCoord) : float4(1, 1, 1, 1);
    OUT.Normal = HasDiffuseNormalEmissive.y > 0.5f ? CalculateNormalMap(IN.Normal.rgb, IN.Tangent.rgb, IN.Bitangent.rgb, IN.TextCoord) : IN.Normal;    
    OUT.Emissive = HasDiffuseNormalEmissive.z > 0.5f ? EmissiveTextureSB.Sample(StaticSampler, IN.TextCoord) : float4(0, 0, 0, 1.0);
    
    float occlusion = HasOcclusionRoughnessMetallicCombined.x > 0.5f ? OcclusionTextureSB.Sample(StaticSampler, IN.TextCoord).r : 0.0f;
    float roughness = 0.0f, metallic = 0.0f;
    
    if (HasOcclusionRoughnessMetallicCombined.w > 0.5f)
    {
        float4 rm_pixel = CombinedTextureSB.Sample(StaticSampler, IN.TextCoord);
        roughness = rm_pixel.g;
        metallic = rm_pixel.b;
    }
    else
    {
        roughness = RoughnessTextureSB.Sample(StaticSampler, IN.TextCoord).r;
        metallic = MetallicTextureSB.Sample(StaticSampler, IN.TextCoord).r;
    }
    
    OUT.ORM = float4(occlusion, roughness, metallic, 1.0f);
    
    return OUT;
}