#include "Shape.h"

#pragma once
class Sphere : public Shape {
	public:
	displib::V3D pos;
	float rad;

	Sphere();

	Sphere(displib::V3D& pos_, float rad_, short col_, bool reflective_);

	float intersectRay(Ray& r) override;

	bool getIntersection(Ray& r, Hit& hitOut) override;
};