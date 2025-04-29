struct PSInput
{
    float4 Position : SV_Position;
    float4 Normal : NORMAL;
    float2 TextCoord : TEXCOORD;
    float4 WorldPos : POSITION;
};

Texture2D DiffuseTextureSB : register(t0);
SamplerState StaticSampler : register(s0);

float4 main(PSInput IN) : SV_Target
{
    float4 color = DiffuseTextureSB.Sample(StaticSampler, IN.TextCoord);
    
    if (color.a < 0.1)
        discard;
    
    float aver = (color.r + color.g + color.b) / 3.0;
    
    if (aver < 0.1)
        discard;
    
    color.a = aver;
    
    return color;
}