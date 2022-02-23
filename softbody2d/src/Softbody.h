#include "Spring.h"

#pragma once
class Softbody {
	public:
	int particleNum=0;
	int springNum=0;
	Particle* particles={};
	Spring* springs={};

	Softbody();

	void formRectangle(int w, int h, AABB2D bodyBnds, float stiff, float damp);

	void formEllipse(int r, int c, AABB2D bodyBnds, float stiff, float damp);

	void formCloth(int w, int h, AABB2D bodyBnds, float stiff, float damp);

	~Softbody();

	void applyForce(V2D f);

	void update(float dt);

	void constrainIn(AABB2D a);

	void constrainOut(AABB2D a);

	void renderParticles(Raster& rst);

	void renderSprings(Raster& rst);
};