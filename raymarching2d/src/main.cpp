#include <iostream>

#include "Engine.h"
#include "maths/Maths.h"
using namespace displib;

//raymarch util funcs
V2D minv(V2D a, V2D b) {
	return V2D(min(a.x, b.x), min(a.y, b.y));
}

V2D maxv(V2D a, V2D b) {
	return V2D(max(a.x, b.x), max(a.y, b.y));
}

V2D absv(V2D v) {
	return V2D(abs(v.x), abs(v.y));
}

struct tri {
	V2D a, b, c;

	float signedDist(V2D pt) {//from inigo quilez
		V2D e0=b-a, e1=c-b, e2=a-c;
		V2D v0=pt-a, v1=pt-b, v2=pt-c;
		V2D pq0=v0-e0*Maths::clamp(v0.dot(e0)/e0.dot(e0), 0, 1);
		V2D pq1=v1-e1*Maths::clamp(v1.dot(e1)/e1.dot(e1), 0, 1);
		V2D pq2=v2-e2*Maths::clamp(v2.dot(e2)/e2.dot(e2), 0, 1);
		float s=Maths::sign(e0.x*e2.y-e0.y*e2.x);
		V2D pc0=V2D(pq0.dot(pq0), s*(v0.x*e0.y-v0.y*e0.x));
		V2D pc1=V2D(pq1.dot(pq1), s*(v1.x*e1.y-v1.y*e1.x));
		V2D pc2=V2D(pq2.dot(pq2), s*(v2.x*e2.y-v2.y*e2.x));
		V2D d=minv(pc0, minv(pc1, pc2));
		return -sqrt(d.x)*Maths::sign(d.y);
	}

	void render(Raster& rst) {
		rst.drawTriangle(a.x, a.y, b.x, b.y, c.x, c.y);
	}
};

struct rect {
	V2D pos;
	float w, h;

	float signedDist(V2D pt_) {//from inigo quilez
		V2D pt=pt_-pos;
		V2D ed=absv(pt)-V2D(w/2, h/2);
		float od=maxv(ed, V2D()).mag();
		float id=min(max(ed.x, ed.y), 0);
		return od+id;
	}

	void render(Raster& rst) {
		rst.drawRect(pos.x-w/2, pos.y-h/2, w, h);
	}
};

struct circle {
	V2D pos;
	float rad;

	float signedDist(V2D pt) {//basic dist func
		V2D sub=pt-pos;
		return sub.mag()-rad;
	}

	void render(Raster& rst) {
		rst.drawCircle(pos.x, pos.y, rad);
	}
};

class Demo : public Engine {
	public:
	tri t;
	circle c;
	rect r;
	V2D ctr;
	float angle=0.0f;

	void setup() override {
		ctr=V2D(width/2, height/2);
		float sz=16;
		t={V2D(0, -sz)+ctr, V2D(sz, sz)+ctr, V2D(-sz, sz)+ctr};
		c={V2D(width/4, height/2), sz};
		r={V2D(width*3/4, height/2), sz, sz*2};
	}

	void update(float dt) override {
		//use keys to change dir
		if (getKey('A'))angle-=Maths::PI*dt;
		if (getKey('D'))angle+=Maths::PI*dt;
	}

	void draw(Raster& rst) override {
		//background
		rst.setChar(' ');
		rst.fillRect(0, 0, width, height);

		//actual "raymarching":
		rst.setChar('.');
		rst.setColor(Raster::CYAN);
		V2D mouseVec=V2D(mouseX, mouseY);
		V2D checkPt=mouseVec;
		V2D dir=V2D::fromAngle(angle);
		float totalDist=0.0f;
		//march until weve gon too far
		while (totalDist<width) {
			float triDist=t.signedDist(checkPt);
			float circDist=c.signedDist(checkPt);
			float rectDist=r.signedDist(checkPt);
			//min of all shapes
			float sceneDist=min(triDist, min(circDist, rectDist));

			//if close to or inside shape, exit
			if (sceneDist<1) break;
			rst.drawCircle(checkPt.x, checkPt.y, sceneDist);

			//add to total dist
			totalDist+=sceneDist;
			//march along
			checkPt+=dir*sceneDist;
		}
		//draw line to final pt
		rst.setChar('#');
		rst.setColor(Raster::RED);
		rst.drawLine(mouseVec.x, mouseVec.y, checkPt.x, checkPt.y);

		//show REAL shapes
		rst.setChar('@');
		rst.setColor(Raster::GREEN);
		t.render(rst);
		c.render(rst);
		r.render(rst);

		//draw final pt
		rst.setChar('p');
		rst.setColor(Raster::WHITE);
		rst.fillRect(checkPt.x-1, checkPt.y-1, 3, 3);

		//show fps
		rst.drawString(0, 0, "FPS: "+std::to_string((int)framesPerSecond));
	}
};

int main() {
	//init custom console engine
	Demo d=Demo();
	d.startFullscreen(6);

	return 0;
}