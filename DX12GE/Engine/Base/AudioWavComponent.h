#pragma once

#include "../Base/DX12LibPCH.h"

struct AudioWavComponent
{
	WAVEFORMATEX WavWFX{};
	std::vector<uint8_t> WavAudioData;

	std::string FilePath;

	bool IsInitialized = false;

	void Destroy()
	{
		WavWFX = WAVEFORMATEX{};
		WavAudioData.clear();
		IsInitialized = false;
	};
};