cbuffer OCB : register(b0)
{
    float4x4 WVP;
    float4x4 MVP;
};

struct VSInput
{
    float3 Position :   POSITION;
    float3 Normal :     NORMAL;
    float2 TextCoord :  TEXCOORD;
    float3 Tangent :    TANGENT;
    float3 Bitangent :  BITANGENT;
};

struct VSOutput
{
    float4 Position :   SV_Position;
    float4 Normal :     NORMAL;
    float2 TextCoord :  TEXCOORD;
    float4 WorldPos :   POSITION0;
    float4 Tangent :    TANGENT;
    float4 Bitangent :  BITANGENT;
};

VSOutput main(VSInput IN)
{
    VSOutput OUT;
    
    OUT.Position = mul(MVP, float4(IN.Position, 1.0f));
    OUT.Normal = normalize(mul(WVP, float4(IN.Normal, 0.0f)));
    OUT.TextCoord = IN.TextCoord;
    OUT.WorldPos = mul(WVP, float4(IN.Position, 1.0f));
    OUT.Tangent = normalize(mul(WVP, float4(IN.Tangent, 0.0f)));
    OUT.Bitangent = normalize(mul(WVP, float4(IN.Bitangent, 0.0f)));
    
    return OUT;
}