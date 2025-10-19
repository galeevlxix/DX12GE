#include "../ShaderResources.h"

static ObjectConstantBuffer* ObjectCB = nullptr;
static ShadowConstantBuffer* ShadowCB = nullptr;
static WorldConstantBuffer* WorldCB = nullptr;
static ParticleConstantBuffer* ParticleCB = nullptr;
static ParticleComputeConstantBuffer* ComputeConstantCB = nullptr;
static BitonicSortConstantBuffer* BitonicCB = nullptr;
static MaterialConstantBuffer* MaterialCB = nullptr;
static SSRConstantBuffer* SSRCB = nullptr;

static UploadBuffer* mPrimaryDeviceUploadBuffer = nullptr;
static UploadBuffer* mSecondDeviceUploadBuffer = nullptr;

static UploadBuffer::Allocation OcbAllocation;
static UploadBuffer::Allocation WcbAllocation;
static UploadBuffer::Allocation ScbAllocation;
static UploadBuffer::Allocation PcbAllocation;
static UploadBuffer::Allocation CcbAllocation;
static UploadBuffer::Allocation BcbAllocation;
static UploadBuffer::Allocation McbAllocation;
static UploadBuffer::Allocation SSRcbAllocation;

static const UINT OcbSize = sizeof(ObjectConstantBuffer);
static const UINT ScbSize = sizeof(ShadowConstantBuffer);
static const UINT WcbSize = sizeof(WorldConstantBuffer);
static const UINT PcbSize = sizeof(ParticleConstantBuffer);
static const UINT CcbSize = sizeof(ParticleComputeConstantBuffer);
static const UINT BcbSize = sizeof(BitonicSortConstantBuffer);
static const UINT McbSize = sizeof(MaterialConstantBuffer);
static const UINT SSRcbSize = sizeof(SSRConstantBuffer);

void ShaderResources::Create(bool singleGpu)
{
	if (!mPrimaryDeviceUploadBuffer || !mSecondDeviceUploadBuffer)
	{
		mPrimaryDeviceUploadBuffer = new UploadBuffer();
		mSecondDeviceUploadBuffer = new UploadBuffer(_2MB, GraphicAdapterSecond);
	}

	ObjectCB =			!ObjectCB ?				new ObjectConstantBuffer() :			ObjectCB;
	ShadowCB =			!ShadowCB ?				new ShadowConstantBuffer() :			ShadowCB;
	WorldCB =			!WorldCB  ?				new WorldConstantBuffer()  :			WorldCB;
	ParticleCB =		!ParticleCB ?			new ParticleConstantBuffer() :			ParticleCB;
	ComputeConstantCB = !ComputeConstantCB ?	new ParticleComputeConstantBuffer() :	ComputeConstantCB;
	BitonicCB =			!BitonicCB ?			new BitonicSortConstantBuffer() :		BitonicCB;
	MaterialCB =		!MaterialCB ?			new MaterialConstantBuffer() :			MaterialCB;
	SSRCB =				!SSRCB ?				new SSRConstantBuffer() :				SSRCB;


}

ObjectConstantBuffer* ShaderResources::GetObjectCB()
{
	return ObjectCB;
}

ShadowConstantBuffer* ShaderResources::GetShadowCB()
{
	return ShadowCB;
}

WorldConstantBuffer* ShaderResources::GetWorldCB()
{
	return WorldCB;
}

ParticleConstantBuffer* ShaderResources::GetParticleCB()
{
	return ParticleCB;
}

ParticleComputeConstantBuffer* ShaderResources::GetParticleComputeCB()
{
	return ComputeConstantCB;
}

BitonicSortConstantBuffer* ShaderResources::GetBitonicSortCB()
{
	return BitonicCB;
}

MaterialConstantBuffer* ShaderResources::GetMaterialCB()
{
	return MaterialCB;
}

SSRConstantBuffer* ShaderResources::GetSSRCB()
{
	return SSRCB;
}

UploadBuffer* ShaderResources::GetUploadBuffer()
{
	return mPrimaryDeviceUploadBuffer;
}

void ShaderResources::SetGraphicsObjectCB(ComPtr<ID3D12GraphicsCommandList2> commandList, uint32_t slot)
{
	OcbAllocation = mPrimaryDeviceUploadBuffer->Allocate(OcbSize, D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT);
	memcpy(OcbAllocation.CPU, ObjectCB, OcbSize);
	commandList->SetGraphicsRootConstantBufferView(slot, OcbAllocation.GPU);
}

void ShaderResources::SetGraphicsShadowCB(ComPtr<ID3D12GraphicsCommandList2> commandList, uint32_t slot)
{
	ScbAllocation = mPrimaryDeviceUploadBuffer->Allocate(ScbSize, D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT);
	memcpy(ScbAllocation.CPU, ShadowCB, ScbSize);
	commandList->SetGraphicsRootConstantBufferView(slot, ScbAllocation.GPU);
}

void ShaderResources::SetGraphicsWorldCB(ComPtr<ID3D12GraphicsCommandList2> commandList, uint32_t slot)
{
	WcbAllocation = mPrimaryDeviceUploadBuffer->Allocate(WcbSize, D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT);
	memcpy(WcbAllocation.CPU, WorldCB, WcbSize);
	commandList->SetGraphicsRootConstantBufferView(slot, WcbAllocation.GPU);
}

void ShaderResources::SetGraphicsParticleCB(ComPtr<ID3D12GraphicsCommandList2> commandList, uint32_t slot)
{
	PcbAllocation = mPrimaryDeviceUploadBuffer->Allocate(PcbSize, D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT);
	memcpy(PcbAllocation.CPU, ParticleCB, PcbSize);
	commandList->SetGraphicsRootConstantBufferView(slot, PcbAllocation.GPU);
}

void ShaderResources::SetParticleComputeCB(ComPtr<ID3D12GraphicsCommandList2> commandList, uint32_t slot)
{
	CcbAllocation = mPrimaryDeviceUploadBuffer->Allocate(CcbSize, D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT);
	memcpy(CcbAllocation.CPU, ComputeConstantCB, CcbSize);
	commandList->SetComputeRootConstantBufferView(slot, CcbAllocation.GPU);
}

void ShaderResources::SetBitonicSortCB(ComPtr<ID3D12GraphicsCommandList2> commandList, uint32_t slot)
{
	BcbAllocation = mPrimaryDeviceUploadBuffer->Allocate(BcbSize, D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT);
	memcpy(BcbAllocation.CPU, BitonicCB, BcbSize);
	commandList->SetComputeRootConstantBufferView(slot, BcbAllocation.GPU);
}

void ShaderResources::SetMaterialCB(ComPtr<ID3D12GraphicsCommandList2> commandList, uint32_t slot)
{
	McbAllocation = mPrimaryDeviceUploadBuffer->Allocate(McbSize, D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT);
	memcpy(McbAllocation.CPU, MaterialCB, McbSize);
	commandList->SetGraphicsRootConstantBufferView(slot, McbAllocation.GPU);
}

void ShaderResources::SetSSRCB(ComPtr<ID3D12GraphicsCommandList2> commandList, uint32_t slot, GraphicsAdapter graphicsAdapter)
{
	SSRcbAllocation = graphicsAdapter == GraphicAdapterPrimary ? 
		mPrimaryDeviceUploadBuffer->Allocate(SSRcbSize, D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT) :
		mSecondDeviceUploadBuffer->Allocate(SSRcbSize, D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT);
	
	memcpy(SSRcbAllocation.CPU, SSRCB, SSRcbSize);
	commandList->SetGraphicsRootConstantBufferView(slot, SSRcbAllocation.GPU);
}

void ShaderResources::Destroy()
{
	delete ObjectCB;
	ObjectCB = nullptr;

	delete ShadowCB;
	ShadowCB = nullptr;

	delete WorldCB;
	WorldCB = nullptr;

	delete ParticleCB;
	ParticleCB = nullptr;

	delete ComputeConstantCB;
	ComputeConstantCB = nullptr;

	delete BitonicCB;
	BitonicCB = nullptr;

	delete MaterialCB;
	MaterialCB = nullptr;

	delete SSRCB;
	SSRCB = nullptr;

	mPrimaryDeviceUploadBuffer->Reset();
	delete mPrimaryDeviceUploadBuffer;
	mPrimaryDeviceUploadBuffer = nullptr;

	mSecondDeviceUploadBuffer->Reset();
	delete mSecondDeviceUploadBuffer;
	mSecondDeviceUploadBuffer = nullptr;
}