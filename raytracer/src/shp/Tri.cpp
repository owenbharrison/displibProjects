#include "Tri.h"

Tri::Tri() {
	this->v0=displib::V3D();
	this->v1=displib::V3D();
	this->v2=displib::V3D();
}

Tri::Tri(displib::V3D& v0_, displib::V3D& v1_, displib::V3D& v2_, short col_, bool reflective_) {
	this->v0=v0_;
	this->v1=v1_;
	this->v2=v2_;
	this->col=col_;
	this->reflective=reflective_;
}

displib::V3D& Tri::getNorm() {
	displib::V3D e1=this->v1-this->v0;
	displib::V3D e2=this->v2-this->v0;
	displib::V3D cr=displib::V3D::normal(e1.cross(e2));
	return cr;
}

float Tri::intersectRay(Ray& r) {
	float EPSILON=0.0001f;
	displib::V3D e1=this->v1-this->v0;
	displib::V3D e2=this->v2-this->v0;
	displib::V3D h=r.dir.cross(e2);
	float n=e1.dot(h);
	if (n>-EPSILON&&n<EPSILON) return -1;
	float f=1/n;
	displib::V3D s=r.origin-this->v0;
	float u=f*s.dot(h);
	if (u<0.0||u>1.0) return -1;
	displib::V3D q=s.cross(e1);
	float v=f*r.dir.dot(q);
	if (v<0.0||u+v>1.0) return -1;
	float t=f*e2.dot(q);
	if (t>EPSILON) return t;
	return -1;
}

bool Tri::getIntersection(Ray& r, Hit& hitOut) {
	float dist=this->intersectRay(r);
	//invalid
	if (dist==-1) return false;

	//use dist to march dir to hitpos
	displib::V3D hitPos=r.origin+r.dir*dist;
	hitOut=Hit(r, dist, hitPos, this->getNorm(), this->col, this->reflective);
	return true;
}