#include "Ray.h"

#pragma once
class Hit {
	public:
	Ray ray;
	float dist;
	displib::V3D pos, norm;
	char col;

	Hit();

	Hit(Ray& ray_, float dist_, displib::V3D& pos_, displib::V3D& norm_, short col_);
};