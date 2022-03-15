//idk how to make this work as an abstract class
#include "Shape.h"

Shape::Shape() {
	this->col=0x000F;
}

float Shape::intersectRay(Ray& r) { return 0; }

bool Shape::getIntersection(Ray& r, Hit* hitOut) { return false; }