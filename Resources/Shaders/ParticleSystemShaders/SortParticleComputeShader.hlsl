cbuffer BitonicCB : register(b0)
{
    uint Level;
    uint LevelMask;
    uint ParticleCount;
    uint Pad;
    float4 CameraPos;
}

struct Particle
{
    float3 Position;
    float3 Speed;
};

RWStructuredBuffer<Particle> ParticleBuffer : register(u0);

float GetDistance(uint sharedParticleIndex)
{
    return length(ParticleBuffer[sharedParticleIndex].Position - CameraPos.xyz);
}

[numthreads(256, 1, 1)]
void main(uint3 dispatchThreadID : SV_DispatchThreadID)
{    
    uint i = dispatchThreadID.x;
    
    if (i >= ParticleCount)
        return;
    
    uint ixj = i ^ LevelMask;

    if (ixj > i)
    {
        Particle a = ParticleBuffer[i];
        Particle b = ParticleBuffer[ixj];

        bool dir = ((i & Level) == 0);

        float aDist = GetDistance(i);
        float bDist = GetDistance(ixj);
        bool shouldSwap = dir ? (aDist < bDist) : (aDist > bDist);

        if (shouldSwap)
        {
            ParticleBuffer[i] = b;
            ParticleBuffer[ixj] = a;
        }
    }
}