#include "Spring.h"

Spring::Spring() {
	this->a=nullptr;
	this->b=nullptr;
	this->restLen=0.0f;
	this->stiff=0.0f;
	this->damp=0.0f;
}

Spring::Spring(Particle& a_, Particle& b_, float stiff_, float damp_) {
	this->a=&a_;
	this->b=&b_;
	this->restLen=(this->getA().pos-this->getB().pos).mag();
	this->stiff=stiff_;
	this->damp=damp_;
}

Particle& Spring::getA() { return *this->a; }
Particle& Spring::getB() { return *this->b; }

V3D Spring::calcForce() {
	V3D sub=this->getB().pos-this->getA().pos;
	V3D dir=V3D::normal(sub);
	float Fs=this->stiff*(sub.mag()-this->restLen);
	float Fd=dir.dot(this->getB().vel-this->getA().vel)*this->damp;
	return dir*(Fs+Fd);
}

void Spring::update() {
	V3D f=this->calcForce();
	this->getA().applyForce(f);
	this->getB().applyForce(f*-1.0f);
}