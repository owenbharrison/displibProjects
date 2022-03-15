#include "maths/vector/V3D.h"

#pragma once
class Ray {
	public:
	displib::V3D origin, dir;

	Ray();

	Ray(displib::V3D& origin_, displib::V3D& dir_);
};