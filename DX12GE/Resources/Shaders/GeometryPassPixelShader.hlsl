struct PSInput
{
    float4 Position :   SV_Position;
    float4 Normal :     NORMAL;
    float2 TextCoord :  TEXCOORD;
    float4 WorldPos :   POSITION0;
};

struct PSOutput
{
    float4 Position :   SV_Target0;
    float4 Normal :     SV_Target1;
    float4 Diffuse :    SV_Target2;
};

Texture2D DiffuseTextureSB : register(t0);
SamplerState StaticSampler : register(s0);

PSOutput main(PSInput IN)
{
    PSOutput OUT;
    
    OUT.Position = IN.WorldPos;
    OUT.Normal = IN.Normal;
    OUT.Diffuse = DiffuseTextureSB.Sample(StaticSampler, IN.TextCoord);
    
    return OUT;
}