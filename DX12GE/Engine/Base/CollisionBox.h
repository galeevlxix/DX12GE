#pragma once
#include "DX12LibPCH.h"

struct CollisionBox
{
	float x_max = 0;
	float x_min = 0;
	float y_max = 0;
	float y_min = 0;
	float z_max = 0;
	float z_min = 0;

	CollisionBox()
	{
		x_max = 0;
		x_min = 0;
		y_max = 0;
		y_min = 0;
		z_max = 0;
		z_min = 0;
	}

	CollisionBox(float xmax, float xmin, float ymax, float ymin, float zmax, float zmin)
	{
		x_max = xmax;
		x_min = xmin;
		y_max = ymax;
		y_min = ymin;
		z_max = zmax;
		z_min = zmin;
	}

	void Add(const DirectX::XMFLOAT3& position)
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

static CollisionBox DefaultCollisionBox = CollisionBox(0.5f, -0.5f, 0.5f, -0.5f, 0.5f, -0.5f);