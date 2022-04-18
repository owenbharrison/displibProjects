#include "Particle.h"
#include <algorithm>

Particle::Particle() {
	this->pos=V2D();
	this->vel=V2D();
	this->acc=V2D();
}

Particle::Particle(V2D pos_, float rad_) {
	this->pos=pos_;
	this->vel=V2D();
	this->acc=V2D();
	this->rad=rad_;
}

void Particle::update(float dt) {
	this->vel+=this->acc*dt;
	this->pos+=this->vel*dt;
	this->acc*=0;
}

void Particle::applyForce(V2D f) {
	this->acc+=f;
}

AABB2D Particle::getAABB() {
	return AABB2D(
		V2D(this->pos.x-this->rad, this->pos.y-this->rad),
		V2D(this->pos.x+this->rad, this->pos.y+this->rad)
	);
}

void Particle::checkAABB(AABB2D a) {
	//bounds detection and bounce
	if (this->pos.x<a.min.x+this->rad) {
		this->pos.x=a.min.x+this->rad;
		this->vel.x*=-1;
	}
	if (this->pos.y<a.min.y+this->rad) {
		this->pos.y=a.min.y+this->rad;
		this->vel.y*=-1;
	}
	if (this->pos.x>a.max.x-this->rad) {
		this->pos.x=a.max.x-this->rad;
		this->vel.x*=-1;
	}
	if (this->pos.y>a.max.y-this->rad) {
		this->pos.y=a.max.y-this->rad;
		this->vel.y*=-1;
	}
}

void Particle::render(Raster& rst) {
	rst.putPixel(this->pos.x, this->pos.y);
}