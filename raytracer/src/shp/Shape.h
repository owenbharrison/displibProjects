#include "../str/Hit.h"

#pragma once
class Shape {
	public:
	short col;

	Shape();

	virtual float intersectRay(Ray& r);

	virtual bool getIntersection(Ray& r, Hit* hitOut);
};