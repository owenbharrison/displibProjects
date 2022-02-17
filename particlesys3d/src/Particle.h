#include "geom/AABB3D.h"
using namespace displib;

#pragma once
class Particle {
	public:
	V3D pos, vel, acc;

	Particle();

	Particle(V3D pos_);

	void update(float dt);

	void applyForce(V3D f);

	void constrainIn(AABB3D a);
};