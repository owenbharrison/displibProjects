#include <vector>

#include "Engine.h"
#include "maths/Maths.h"
#include "geom/AABB2D.h"
using namespace displib;

#define SS 10

#define STIFF 360.45f
#define DAMP 5.7f

//thanks to pezzza for verlet stuff!
//https://www.youtube.com/watch?v=lS_qeBy3aQI&t=181s&ab_channel=Pezzza%27sWork
struct point {
	float2 pos, oldpos, acc;
	float rad=0;

	point() {}

	point(float2 pos_, float rad_) : pos(pos_), oldpos(pos_), rad(rad_) {}

	void update(float dt) {
		float2 vel=pos-oldpos;
		oldpos=pos;

		pos+=vel+acc*dt*dt;

		acc*=0;
	}

	void accelerate(float2 force) {
		acc+=force;
	}

	void checkAABB(AABB2D a) {
		if (pos.x<a.min.x+rad) {
			pos.x=a.min.x+rad;
		}
		if (pos.y<a.min.y+rad) {
			pos.y=a.min.y+rad;
		}
		if (pos.x>a.max.x-rad) {
			pos.x=a.max.x-rad;
		}
		if (pos.y>a.max.y-rad) {
			pos.y=a.max.y-rad;
		}
	}
};

//spring with "infinite" stiffness?
struct stick {
	point* a=nullptr, * b=nullptr;
	float restLen=0;

	stick() {}

	stick(point& a_, point& b_) : a(&a_), b(&b_) {
		restLen=length(a->pos-b->pos);
	}

	void update() {
		float2 axis=a->pos-b->pos;
		float dist=length(axis);
		float2 n=axis/dist;
		float delta=restLen-dist;
		a->pos+=n*delta*.5;
		b->pos-=n*delta*.5;
	}
};

struct spring {
	point* a=nullptr, * b=nullptr;
	float restLen=0, stiff=0, damp=0;

	spring() {}

	spring(point& a_, point& b_, float stiff_, float damp_) : a(&a_), b(&b_), stiff(stiff_), damp(damp_) {
		restLen=length(a->pos-b->pos);
	}

	void update() {
		//f=-cv-kx
		float2 sub=b->pos-a->pos;
		float diff=length(sub)-restLen;
		float2 tang=normalize(sub);
		float2 a_cv=-damp*(a->pos-a->oldpos);
		float2 b_cv=-damp*(b->pos-b->oldpos);
		float2 kx=stiff*(tang*diff/2);
		a->accelerate(a_cv+kx);
		b->accelerate(a_cv-kx);
	}
};

class Demo : public Engine {
	public:
	float2 grav;
	AABB2D bounds;

	std::vector<point> points;
	std::vector<stick> sticks;
	std::vector<spring> springs;

	void setup() override {
		grav=float2(0, 32);

		bounds=AABB2D(0, 0, width, height);

		auto sz=20;
		float2 mid(width/2, height/2);
		points={
			point(mid-float2(sz), 2),
			point(mid+float2(sz, -sz), 2),
			point(mid+float2(sz), 2),
			point(mid+float2(-sz, sz), 2),
			point(mid+float2(sz, sz*2), 2)
		};

		springs={
			spring(points[0], points[1], STIFF, DAMP),
			spring(points[1], points[2], STIFF, DAMP),
			spring(points[3], points[0], STIFF, DAMP),
			spring(points[0], points[2], STIFF, DAMP)
		};

		sticks={
			stick(points[2], points[3]),
			stick(points[2], points[4]),
			stick(points[3], points[4])
		};
	}

	void update(float dt) override {
		//update springs
		for (spring& s:springs) {
			s.update();
		}
		//update points
		for (point& p:points) {
			p.accelerate(grav);

			p.update(dt);
		}
		//loop
		for (int i=0; i<SS; i++) {
			//update sticks
			for (stick& s:sticks) {
				s.update();
			}
			//constrain points
			for (point& p:points) {
				p.checkAABB(bounds);
			}
		}
	}

	void draw(Raster& rst) override {
		rst.setChar(' ');
		rst.fillRect(0, 0, width, height);

		rst.setChar('#');
		for (stick& s:sticks) {
			rst.drawLine(s.a->pos, s.b->pos);
		}

		rst.setChar('~');
		for (spring& s:springs) {
			rst.drawLine(s.a->pos, s.b->pos);
		}

		rst.setChar(0x2588);
		for (point& p:points) {
			rst.fillCircle(p.pos, p.rad);
		}
	}
};

int main() {
	srand(time(NULL));

	//init custom graphics engine
	Demo d;
	d.startFullscreen(6);

	return 0;
}