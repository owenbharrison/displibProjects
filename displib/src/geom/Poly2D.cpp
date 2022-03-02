#include "Poly2D.h"

namespace displib {
	Poly2D::Poly2D() : Poly2D::Poly2D(V2D(), nullptr, 0) {}

	Poly2D::Poly2D(V2D pos_, V2D* pts, int num) {
		this->pos=pos_;
		this->angle=0.0f;
		this->numPts=num;
		this->model=pts;
		this->points=new V2D[this->numPts];
		this->updatePoints();
	}

	Poly2D::Poly2D(V2D pos_, float rad, int num) {
		this->pos=pos_;
		this->angle=0.0f;
		this->numPts=num;
		this->model=new V2D[this->numPts];
		for (int i=0; i<this->numPts; i++) {
			float angle =Maths::map(i*1.0f, 0.0f, 1.0f*this->numPts, 0.0f, Maths::TAU);
			this->model[i]=V2D::fromAngle(angle)*rad;
		}
		this->points=new V2D[this->numPts];
		this->updatePoints();
	}

	void Poly2D::updatePoints() {
		for (int i=0; i<this->numPts; i++) {
			//special formula
			V2D mp=this->model[i];
			float sa=sinf(angle);
			float ca=cosf(angle);
			this->points[i]=V2D(mp.x*ca-mp.y*sa, mp.y*ca+mp.x*sa)+this->pos;
		}
	}

	AABB2D Poly2D::getAABB() {
		float nx=INFINITY;
		float ny=INFINITY;
		float mx=-INFINITY;
		float my=-INFINITY;
		for (int i=0; i<this->numPts; i++) {//sort
			V2D pt=this->points[i];
			nx=min(nx, pt.x);
			ny=min(ny, pt.y);
			mx=max(mx, pt.x);
			my=max(my, pt.y);
		}
		return AABB2D(nx, ny, mx, my);
	}

	bool Poly2D::containsPt(V2D pt) {
		if (this->getAABB().containsPt(pt)) {
			V2D pte=pt+V2D(1, 0);
			int numIx=0;
			for (int i=0; i<this->numPts; i++) {
				V2D a=this->points[i];
				V2D b=this->points[(i+1)%this->numPts];
				float* ix=Maths::lineLineIntersection(a, b, pt, pte);
				if (ix[0]>0&&ix[0]<1&&ix[1]>0) {//ray intersect
					numIx++;
				}
			}
			return numIx%2==1;
		}
		return false;
	}

	void Poly2D::render(Raster rst) {
		for (int i=0; i<this->numPts; i++) {
			V2D a=this->points[i];
			V2D b=this->points[(i+1)%this->numPts];
			rst.drawLine((int)a.x, (int)a.y, (int)b.x, (int)b.y);
		}
	}
}