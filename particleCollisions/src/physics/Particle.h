#include "geom/AABB2D.h"
using namespace displib;

#pragma once
class Particle {
	public:
	V2D pos, vel, acc;
	float rad;

	Particle();

	Particle(V2D pos_, float rad_);

	void update(float dt);

	void applyForce(V2D f);

	AABB2D getAABB();

	void checkAABB(AABB2D a);

	void render(Raster& rst);
};