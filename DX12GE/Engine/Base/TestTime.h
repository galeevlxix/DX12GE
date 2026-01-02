#pragma once

#include "Singleton.h"
#include <chrono>
#include <vector>
#include <fstream>
#include <string>

using namespace std::chrono;

class TestTime
{
	const std::string s_path = "../../DX12GE/Resources/single_time.txt";
	const std::string m_path = "../../DX12GE/Resources/multi_time.txt";

	int frameCounter;
	int maxFrames = 100;

	std::map<CurrentPass::EPass, double> FramePassTime;
	std::map<CurrentPass::EPass, double> TotalPassTime;

	std::vector<double> frameDurations;
	double totalDuration;

	steady_clock::time_point beginPass;
	steady_clock::time_point beginFrame;

	bool finished;

public:
	TestTime()
	{
		frameCounter = 0;
		finished = false;
		totalDuration = 0.0;
	}

	void BeginFrame()
	{
		if (finished) return;
		frameCounter++;

		beginFrame = steady_clock::now();
	}

	void EndFrame()
	{
		if (finished) return;

		for (auto t : FramePassTime)
		{
			if (!TotalPassTime.contains(t.first))
			{
				TotalPassTime[t.first] = 0.0;
			}
			TotalPassTime[t.first] += t.second;
 		}

		auto endFrame = steady_clock::now();
		double frameTime = duration<double>(endFrame - beginFrame).count();
		totalDuration += frameTime;
		frameDurations.push_back(frameTime);

		if (frameCounter == maxFrames)
		{
			for (auto t : TotalPassTime)
			{
				TotalPassTime[t.first] /= (double)maxFrames;
			}

			totalDuration /= (double)maxFrames;

			finished = true;
			PrintTotal();
		}
	}

	void BeginPass()
	{
		if (finished) return;

		beginPass = steady_clock::now();
	}

	void EndPass(CurrentPass::EPass pass)
	{
		if (finished) return;

		auto endPass = steady_clock::now();
		double passTime = duration<double>(endPass - beginPass).count();
		FramePassTime[pass] = passTime;
	}

private:
	void PrintTotal()
	{
		printf("Среднее время: %f sec\n", totalDuration);

		std::string path = s_path;

		for (auto t : TotalPassTime)
		{
			printf("	%s: %f sec\n", EPassNames[t.first], t.second);

			if (EPassNames[t.first] == "CopyPrimaryDeviceDataToSharedMemory")
			{
				path = m_path;
			}
		}		

		std::ofstream out;
		out.open(path);

		for (auto t : frameDurations)
		{
			out << to_string(t) << std::endl;
		}	

		out.close();
	}
};