cbuffer PCB : register(b0)
{
    float3 BoxPos;
    float DeltaTime;
    float3 BoxSize;
    float ParticleCount;
};

struct Particle
{
    float3 Position;
    float3 Speed;
};

RWStructuredBuffer<Particle> ParticleBuffer : register(u0);
Texture3D VelocityTexture3D : register(t0);
SamplerState StaticSampler : register(s0);

[numthreads(256, 1, 1)]
void main(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    int globalIndex = dispatchThreadID.x;
    
    if (globalIndex >= ParticleCount)
        return;
    
    Particle particle = ParticleBuffer[globalIndex];
    float3 coords = (particle.Position - BoxPos) / BoxSize;
    coords.x = clamp(coords.x, 0.0, 1.0);
    coords.y = clamp(coords.y, 0.0, 1.0);
    coords.z = clamp(coords.z, 0.0, 1.0);

    float4 velocityVector = VelocityTexture3D.SampleLevel(StaticSampler, coords, 0);
    particle.Speed += velocityVector.xyz * DeltaTime;
    particle.Position += particle.Speed * DeltaTime;

    ParticleBuffer[globalIndex] = particle;
}