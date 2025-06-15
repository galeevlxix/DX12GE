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
		None = 7
	};

	static EPass Get();
	static void Set(EPass pass);
};