#include "Shape.h"

#pragma once
class Tri : public Shape {
	public:
	displib::V3D v0, v1, v2;

	Tri();

	Tri(displib::V3D& v0_, displib::V3D& v1_, displib::V3D& v2_, short col_, bool reflective_);

	float intersectRay(Ray& r) override;

	displib::V3D& getNorm();

	bool getIntersection(Ray& r, Hit& hitOut) override;
};