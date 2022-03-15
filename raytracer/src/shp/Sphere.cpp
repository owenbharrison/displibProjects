#include "Sphere.h"

Sphere::Sphere() {
	this->pos=displib::V3D();
	this->rad=0;
	this->col=0x000F;
}

Sphere::Sphere(displib::V3D& pos_, float rad_, short col_) {
	this->pos=pos_;
	this->rad=rad_;
	this->col=col_;
}

//adapted from https://viclw17.github.io/2018/07/16/raytracing-ray-sphere-intersection
float Sphere::intersectRay(Ray& r) {
	float EPSILON=0.0001f;
	displib::V3D oc=r.origin-this->pos;
	float a=r.dir.dot(r.dir);
	float b=2*oc.dot(r.dir);
	float c=oc.dot(oc)-this->rad*this->rad;
	float disc=b*b-4*a*c;
	if (disc<EPSILON) return -1;
	//solve quadratic, + & -
	float sqrt=sqrtf(disc);
	float num=-b-sqrt;
	if (num>EPSILON) return num/(2*a);
	num=-b+sqrt;
	if (num>EPSILON) return num/(2*a);
	return -1;
}

bool Sphere::getIntersection(Ray& r, Hit* hitOut) {
	float dist=this->intersectRay(r);
	//invalid
	if (dist==-1) return false;

	//use dist to march dir to hitpos
	displib::V3D hitPos=r.origin+r.dir*dist;
	//to get norm of UNIFORM surface
	displib::V3D hitNorm=displib::V3D::normal(hitPos-pos);
	*hitOut=Hit(r, dist, hitPos, hitNorm, this->col);
	return true;
}