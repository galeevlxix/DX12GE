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
		None = 8
	};

	EPass Get();
	void Set(EPass pass);

private:
	EPass Pass = CurrentPass::None;
};