cbuffer OCB : register(b0)
{
    matrix ViewProjM;
    float4 CameraPos;
};

struct VSOutput
{
    float4 Position : SV_Position;
    float4 Velocity : VELOCITY;
};

struct GSOutput
{
    float4 Position : SV_Position;
    float4 Normal : NORMAL;
    float2 TextCoord : TEXCOORD;
    float4 WorldPos : POSITION;
};

Texture3D VelocityTexture3D : register(t1);
SamplerState StaticSampler : register(s0);
    
[maxvertexcount(4)]
void main(point VSOutput gsIn[1], uint primId : SV_PrimitiveID, inout TriangleStream<GSOutput> triStream)
{
    float3 pos = gsIn[0].Position.xyz;
    
    float3 toCamera = CameraPos.xyz - pos;
    toCamera = normalize(toCamera);
    float3 up = float3(0.0, 1.0, 0.0);
    
    float3 right = cross(toCamera, up);
    right = normalize(right);
    float3 top = cross(right, toCamera);
    top = normalize(top);
    
    float4 v[4];
    v[0] = float4(pos + 0.5f * right - 0.5f * top, 1.0f);
    v[1] = float4(pos + 0.5f * right + 0.5f * top, 1.0f);
    v[2] = float4(pos - 0.5f * right - 0.5f * top, 1.0f);
    v[3] = float4(pos - 0.5f * right + 0.5f * top, 1.0f);
    
    float2 texC[4] =
    {
        float2(0.0f, 1.0f),
		float2(0.0f, 0.0f),
		float2(1.0f, 1.0f),
		float2(1.0f, 0.0f)
    };
    
    GSOutput gsOut;
    [unroll]
    for (int i = 0; i < 4; i++)
    {
        gsOut.Position = mul(ViewProjM, v[i]);
        gsOut.WorldPos = v[i];
        gsOut.Normal = float4(toCamera, 0.0);
        gsOut.TextCoord = texC[i];
        triStream.Append(gsOut);
    }
}