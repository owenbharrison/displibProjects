#include "Particle.h"

#pragma once
class Spring {
	private:
	Particle* a, * b;

	public:
	float restLen, stiff, damp;

	Spring();

	Spring(Particle& a_, Particle& b_, float stiff_, float damp_);

	Particle& getA();
	Particle& getB();

	V3D calcForce();

	void update();
};