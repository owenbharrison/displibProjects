#include <iostream>

#include "Engine.h"
#include "maths/Maths.h"
using namespace displib;

//raymarch util funcs
float2 minv(float2 a, float2 b) {
	return float2(min(a.x, b.x), min(a.y, b.y));
}

float2 maxv(float2 a, float2 b) {
	return float2(max(a.x, b.x), max(a.y, b.y));
}

float2 absv(float2 v) {
	return float2(abs(v.x), abs(v.y));
}

struct tri {
	float2 a, b, c;

	float signedDist(float2 pt) {//from inigo quilez
		float2 e0=b-a, e1=c-b, e2=a-c;
		float2 v0=pt-a, v1=pt-b, v2=pt-c;
		float2 pq0=v0-e0*Maths::clamp(dot(v0, e0)/dot(e0, e0), 0, 1);
		float2 pq1=v1-e1*Maths::clamp(dot(v1, e1)/dot(e1, e1), 0, 1);
		float2 pq2=v2-e2*Maths::clamp(dot(v2, e2)/dot(e2, e2), 0, 1);
		float s=Maths::sign(e0.x*e2.y-e0.y*e2.x);
		float2 pc0=float2(dot(pq0, pq0), s*(v0.x*e0.y-v0.y*e0.x));
		float2 pc1=float2(dot(pq1, pq1), s*(v1.x*e1.y-v1.y*e1.x));
		float2 pc2=float2(dot(pq2, pq2), s*(v2.x*e2.y-v2.y*e2.x));
		float2 d=minv(pc0, minv(pc1, pc2));
		return -sqrt(d.x)*Maths::sign(d.y);
	}

	void render(Raster& rst) {
		rst.drawTriangle(a, b, c);
	}
};

struct rect {
	float2 pos;
	float w, h;

	float signedDist(float2 pt_) {//from inigo quilez
		float2 pt=pt_-pos;
		float2 ed=absv(pt)-float2(w/2, h/2);
		float od=length(maxv(ed, float2()));
		float id=min(max(ed.x, ed.y), 0);
		return od+id;
	}

	void render(Raster& rst) {
		rst.drawRect(pos.x-w/2, pos.y-h/2, w, h);
	}
};

struct circle {
	float2 pos;
	float rad;

	float signedDist(float2 pt) {//basic dist func
		float2 sub=pt-pos;
		return length(sub)-rad;
	}

	void render(Raster& rst) {
		rst.drawCircle(pos, rad);
	}
};

class Demo : public Engine {
	public:
	tri t;
	circle c;
	rect r;
	float2 ctr;
	float angle=0.0f;

	void setup() override {
		ctr=float2(width/2, height/2);
		float sz=16;
		t={float2(0, -sz)+ctr, float2(sz, sz)+ctr, float2(-sz, sz)+ctr};
		c={float2(width/4, height/2), sz};
		r={float2(width*3/4, height/2), sz, sz*2};
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
		float2 mouseVec=float2(mouseX, mouseY);
		float2 checkPt=mouseVec;
		float2 dir=float2(cosf(angle), sinf(angle));
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
			rst.drawCircle(checkPt, sceneDist);

			//add to total dist
			totalDist+=sceneDist;
			//march along
			checkPt+=dir*sceneDist;
		}
		//draw line to final pt
		rst.setChar('#');
		rst.setColor(Raster::RED);
		rst.drawLine(mouseVec, checkPt);

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