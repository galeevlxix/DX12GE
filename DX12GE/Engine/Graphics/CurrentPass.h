#pragma once

class CurrentPass
{
public:
	enum EPass
	{
		Shadow = 0,
		Geometry = 1,
		Lighting = 2,
		SSR = 3,
		Merging = 4,
		TransparentParticles = 5,
		Debug = 6,
		Skybox = 7,
		CopyPrimaryDeviceDataToSharedMemory = 8,
		CopySharedMemoryDataToPrimaryDevice = 9,
		CopySecondDeviceDataToSharedMemory = 10,
		CopySharedMemoryDataToSecondDevice = 11,

		WaitPrimaryDeviceDataToSharedMemory = 12,
		WaitSharedMemoryDataToPrimaryDevice = 13,
		WaitSecondDeviceDataToSharedMemory = 14,
		WaitSharedMemoryDataToSecondDevice = 15,

		None = 16
	};

	EPass Get();
	void Set(EPass pass);

private:
	EPass Pass = CurrentPass::None;
};

static const char* EPassNames[] =
{
	"Shadow",
	"Geometry",
	"Lighting",
	"SSR",
	"Merging",
	"TransparentParticles",
	"Debug",
	"Skybox",
	"CopyPrimaryDeviceDataToSharedMemory",
	"CopySharedMemoryDataToPrimaryDevice",
	"CopySecondDeviceDataToSharedMemory",
	"CopySharedMemoryDataToSecondDevice",
	"WaitPrimaryDeviceDataToSharedMemory",
	"WaitSharedMemoryDataToPrimaryDevice",
	"WaitSecondDeviceDataToSharedMemory",
	"WaitSharedMemoryDataToSecondDevice",
	"None"
};