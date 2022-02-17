#include "Particle.h"

Particle::Particle() {
	this->pos=V3D();
	this->vel=V3D();
	this->acc=V3D();
}

Particle::Particle(V3D pos_) {
	this->pos=pos_;
	this->vel=V3D();
	this->acc=V3D();
}

void Particle::update(float dt) {
	this->vel+=this->acc*dt;
	this->pos+=this->vel*dt;
	this->acc*=0.0f;
}

void Particle::applyForce(V3D f) {
	this->acc+=f;
}

void Particle::constrainIn(AABB3D a) {
	//bounds detection and bounce
	if (this->pos.x<a.min.x) {
		this->pos.x=a.min.x;
		this->vel.x*=-1.0f;
	}
	if (this->pos.y<a.min.y) {
		this->pos.y=a.min.y;
		this->vel.y*=-1.0f;
	}
	if (this->pos.z<a.min.z) {
		this->pos.z=a.min.z;
		this->vel.z*=-1.0f;
	}
	if (this->pos.x>a.max.x) {
		this->pos.x=a.max.x;
		this->vel.x*=-1.0f;
	}
	if (this->pos.y>a.max.y) {
		this->pos.y=a.max.y;
		this->vel.y*=-1.0f;
	}
	if (this->pos.z>a.max.z) {
		this->pos.z=a.max.z;
		this->vel.z*=-1.0f;
	}
}