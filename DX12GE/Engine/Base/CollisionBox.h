#pragma once
#include "SimpleMath.h"

using namespace DirectX::SimpleMath;

struct CollisionBox
{
	float x_max = 0;
	float x_min = 0;
	float y_max = 0;
	float y_min = 0;
	float z_max = 0;
	float z_min = 0;

	void Add(XMFLOAT3 position)
	{
		if (isEmpty)
		{
			x_max = position.x;
			x_min = position.x;
			y_max = position.y;
			y_min = position.y;
			z_max = position.z;
			z_min = position.z;
			isEmpty = false;
			return;
		}

		x_max = position.x > x_max ? position.x : x_max;
		x_min = position.x < x_min ? position.x : x_min;

		y_max = position.y > y_max ? position.y : y_max;
		y_min = position.y < y_min ? position.y : y_min;

		z_max = position.z > z_max ? position.z : z_max;
		z_min = position.z < z_min ? position.z : z_min;
	}

private:
	bool isEmpty = true;
};