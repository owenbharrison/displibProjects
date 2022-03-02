#include "Particle.h"
#include <algorithm>

Particle::Particle() {
	this->pos=V2D();
	this->vel=V2D();
	this->acc=V2D();
}

Particle::Particle(V2D pos_) {
	this->pos=pos_;
	this->vel=V2D();
	this->acc=V2D();
}

void Particle::update(float dt) {
	this->vel+=this->acc*dt;
	this->pos+=this->vel*dt;
	this->acc*=0.0f;
}

void Particle::applyForce(V2D f) {
	if(!this->locked) this->acc+=f;
}

void Particle::constrainIn(AABB2D a) {
	//bounds detection and bounce
	if (this->pos.x<a.min.x) {
		this->pos.x=a.min.x;
		this->vel.x*=-1.0f;
	}
	if (this->pos.y<a.min.y) {
		this->pos.y=a.min.y;
		this->vel.y*=-1.0f;
	}
	if (this->pos.x>a.max.x) {
		this->pos.x=a.max.x;
		this->vel.x*=-1.0f;
	}
	if (this->pos.y>a.max.y) {
		this->pos.y=a.max.y;
		this->vel.y*=-1.0f;
	}
}

void Particle::constrainOut(AABB2D a) {
	if (a.containsPt(this->pos)){
		//bounds detection and bounce
		float dnx=abs(a.min.x-this->pos.x);
		float dny=abs(a.min.y-this->pos.y);
		float dmx=abs(a.max.x-this->pos.x);
		float dmy=abs(a.max.y-this->pos.y);
		float cl=min(dnx, min(dny, min(dmx, dmy)));
		if (cl==dnx) {
			this->pos.x=a.min.x;
				this->vel.x*=-1.0f;
		}
		if (cl==dny) {
			this->pos.y=a.min.y;
			this->vel.y*=-1.0f;
		}
		if (cl==dmx) {
			this->pos.x=a.max.x;
			this->vel.x*=-1.0f;
		}
		if (cl==dmy) {
			this->pos.y=a.max.y;
			this->vel.y*=-1.0f;
		}
	}
}

void Particle::render(Raster& rst) {
	rst.putPixel(this->pos.x, this->pos.y);
}