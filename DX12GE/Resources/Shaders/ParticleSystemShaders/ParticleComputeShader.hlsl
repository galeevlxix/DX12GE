cbuffer PCB : register(b0)
{
    float3 BoxPos;
    float DeltaTime;
    float3 BoxSize;
    float ParticleCount;
    float3 CameraPos;
    float Mode;
};

struct Particle
{
    float3 Position;
    float3 Speed;
};

RWStructuredBuffer<Particle> ParticleBuffer : register(u0);
Texture3D VelocityTexture3D : register(t0);
SamplerState StaticSampler : register(s0);

// Локальный буфер для хранения группы данных в общей памяти
groupshared Particle sharedParticles[256];

float GetDistance(int sharedParticleIndex)
{
    return length(sharedParticles[sharedParticleIndex].Position - CameraPos);
}

[numthreads(256, 1, 1)]
void main(uint3 dispatchThreadID : SV_DispatchThreadID,
            uint3 groupThreadId : SV_GroupThreadID)
{
    uint globalIndex = dispatchThreadID.x;
    
    if (globalIndex >= ParticleCount)
        return;
    
    if (Mode < 0.5)
    {
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
    else
    {
        // Загружаем данные из глобальной памяти в shared память для сортировки
        sharedParticles[groupThreadId.x] = ParticleBuffer[globalIndex];
        GroupMemoryBarrierWithGroupSync();
        
        // Битоническая сортировка:
        // k - размер сортируемого subsequence, j - шаг сравнения.
        for (uint k = 2; k <= 256; k *= 2)
        {
            for (uint j = k / 2; j > 0; j /= 2)
            {
                uint ixj = groupThreadId.x ^ j;

                if (ixj > groupThreadId.x)
                {
                    bool condition;
                    if ((groupThreadId.x & k) == 0)
                    {
                    // Сортировка по убыванию: если текущая частица имеет меньший distance, чем "побратим"
                        condition = (GetDistance(groupThreadId.x) < GetDistance(ixj));
                    }
                    else
                    {
                        condition = (sharedParticles[groupThreadId.x].distance > sharedParticles[ixj].distance);
                    }

                // Если условие выполняется, обмениваем элементы
                    if (condition)
                    {
                        Particle temp = sharedParticles[groupThreadId.x];
                        sharedParticles[groupThreadId.x] = sharedParticles[ixj];
                        sharedParticles[ixj] = temp;
                    }
                }
            // Синхронизация группы, чтобы обновлённые данные были видны для всех потоков.
                GroupMemoryBarrierWithGroupSync();
            }
        }

    // Записываем отсортированные данные обратно в глобальный буфер
        particles[globalIndex] = sharedParticles[groupThreadId.x];
    }
    
}