#include "Softbody.h"
#include "maths/Maths.h"

Softbody::Softbody() {//basic
	this->particleNum=0;
	this->springNum=0;
	this->particles={};
	this->springs={};
}

void Softbody::formRectangle(int w, int h, AABB2D bodyBnds, float stiff, float damp) {
	//2d array index maths
	auto ix=[&](int i, int j) { return i+j*w; };

	/*what the structure looks like:
	0 is particle
	-, |, X are springs

	0-0-0
	|X|X|
	0-0-0
	|X|X|
	0-0-0

	*/

	//simple 2d array maths
	this->particleNum=w*h;
	this->particles=new Particle[this->particleNum];
	//spawn particles, 0 in the diagram
	for (int i=0; i<w; i++) {
		for (int j=0; j<h; j++) {
			//evenly distribute
			float x=Maths::map(i, 0, w-1, bodyBnds.min.x, bodyBnds.max.x);
			float y=Maths::map(j, 0, h-1, bodyBnds.min.y, bodyBnds.max.y);
			this->particles[ix(i, j)]=Particle(V2D(x, y));
		}
	}

	//connect axis aligned springs, the - and | in the diagram
	int k=0;
	//complex index maths worked out by hand
	this->springNum=4*w*h-3*w-3*h+2;
	this->springs=new Spring[this->springNum];
	for (int i=0; i<w; i++) {
		for (int j=0; j<h; j++) {
			//connect right if can
			if (i<w-1) this->springs[k++]=Spring(this->particles[ix(i, j)], this->particles[ix(i+1, j)], stiff, damp);
			//connect down if can
			if (j<h-1) this->springs[k++]=Spring(this->particles[ix(i, j)], this->particles[ix(i, j+1)], stiff, damp);
		}
	}
	//connect diags springs, the Xs in the diagram
	for (int i=0; i<w-1; i++) {
		for (int j=0; j<h-1; j++) {
			//connect to opposite corners
			this->springs[k++]=Spring(this->particles[ix(i, j)], this->particles[ix(i+1, j+1)], stiff, damp);
			this->springs[k++]=Spring(this->particles[ix(i+1, j)], this->particles[ix(i, j+1)], stiff, damp);
		}
	}
}

void Softbody::formEllipse(int r, int c, AABB2D bodyBnds, float stiff, float damp) {
	//custom circular index maths
	auto ix=[&](int i, int j) {
		if (i==0)return 0;
		if (j==0)return i;
		return i+j*(r-1);
	};

	this->particleNum=c*r-c+1;
	this->particles=new Particle[this->particleNum];
	//spawn particles, 0 in the diagram
	V2D sz=(bodyBnds.max-bodyBnds.min)/2;
	V2D ctr=bodyBnds.min+sz;
	for (int i=0; i<r; i++) {
		for (int j=0; j<c; j++) {
			//polar to cartesian with ellipse?
			float angle=((float)j/(float)c)*Maths::TAU;
			float t=(float)i/(float)(r-1);
			V2D dir=V2D::fromAngle(angle);
			V2D hullPt(sz.x*dir.x, sz.y*dir.y);
			this->particles[ix(i, j)]=Particle(hullPt*t+ctr);
		}
	}

	int k=0;
	//complex index maths worked out by hand
	this->springNum=2*c*(2*r-3);
	//radially aligned (tangential and norm)
	this->springs=new Spring[this->springNum];
	for (int i=1; i<r; i++) {
		for (int j=0; j<c; j++) {
			//in circle, left
			this->springs[k++]=Spring(this->particles[ix(i, j)], this->particles[ix(i, (j+1)%c)], stiff, damp);
			//in rad, down
			this->springs[k++]=Spring(this->particles[ix(i, j)], this->particles[ix(i-1, j)], stiff, damp);
		}
	}

	//diags
	for (int i=1; i<r-1; i++) {
		for (int j=0; j<c; j++) {
			//this->[left down]
			this->springs[k++]=Spring(this->particles[ix(i, j)], this->particles[ix(i+1, (j+1)%c)], stiff, damp);
			//down->left
			this->springs[k++]=Spring(this->particles[ix(i+1, j)], this->particles[ix(i, (j+1)%c)], stiff, damp);
		}
	}
}

void Softbody::formCloth(int w, int h, AABB2D bodyBnds, float stiff, float damp) {
	//2d array index maths
	auto ix=[&](int i, int j) { return i+j*w; };

	/*what the structure looks like:
	0 is particle, 1, means locked from moving
	-, |

	1-1-1
	| | |
	0-0-0
	| | |
	0-0-0

	*/

	//simple 2d array maths
	this->particleNum=w*h;
	this->particles=new Particle[this->particleNum];
	//spawn particles, 0 in the diagram
	for (int i=0; i<w; i++) {
		for (int j=0; j<h; j++) {
			//evenly distribute
			float x=Maths::map(i, 0, w-1, bodyBnds.min.x, bodyBnds.max.x);
			float y=Maths::map(j, 0, h-1, bodyBnds.min.y, bodyBnds.max.y);
			Particle p=Particle(V2D(x, y));
			p.locked=j==0;
			this->particles[ix(i, j)]=p;
		}
	}

	//connect axis aligned springs, the - and | in the diagram
	this->springNum=2*w*h-w-h;
	this->springs=new Spring[this->springNum];
	for (int i=0, k=0; i<w; i++) {
		for (int j=0; j<h; j++) {
			//connect right if can
			if (i<w-1) this->springs[k++]=Spring(this->particles[ix(i, j)], this->particles[ix(i+1, j)], stiff, damp);
			//connect down if can
			if (j<h-1) this->springs[k++]=Spring(this->particles[ix(i, j)], this->particles[ix(i, j+1)], stiff, damp);
		}
	}
}

Softbody::~Softbody() {
	delete[] this->particles;
	delete[] this->springs;
}

void Softbody::applyForce(V2D f) {
	for (int i=0; i<this->particleNum; i++) {
		this->particles[i].applyForce(f);
	}
}

void Softbody::update(float dt) {
	for (int i=0; i<this->springNum; i++) {
		this->springs[i].update();
	}

	for (int i=0; i<this->particleNum; i++) {
		this->particles[i].update(dt);
	}
}

void Softbody::constrainIn(AABB2D a) {
	for (int i=0; i<this->particleNum; i++) {
		this->particles[i].constrainIn(a);
	}
}

void Softbody::constrainOut(AABB2D a) {
	for (int i=0; i<this->particleNum; i++) {
		this->particles[i].constrainOut(a);
	}
}

void Softbody::renderParticles(Raster& rst) {
	for (int i=0; i<this->particleNum; i++) {
		this->particles[i].render(rst);
	}
}

void Softbody::renderSprings(Raster& rst) {
	for (int i=0; i<this->springNum; i++) {
		this->springs[i].render(rst);
	}
}