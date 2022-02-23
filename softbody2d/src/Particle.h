#include "geom/AABB2D.h"
using namespace displib;

#pragma once
class Particle {
	public:
	V2D pos, vel, acc;
	bool locked=false;

	Particle();

	Particle(V2D pos_);

	void update(float dt);

	void applyForce(V2D f);

	void constrainIn(AABB2D a);

	void constrainOut(AABB2D a);

	void render(Raster& rst);
};