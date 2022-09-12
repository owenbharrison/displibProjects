#include <vector>

#include "Engine.h"
#include "maths/Maths.h"
#include "maths/vector/float3.h"
using namespace displib;

#define EPSILON 0.0001f

int sign(float f) {
	return f>0?1:f<0?-1:0;
}

float dot2(float3 v) { return dot(v, v); }

float3 vmax(float3 a, float3 b) {
	return float3(
		max(a.x, b.x),
		max(a.y, b.y),
		max(a.z, b.z)
	);
}

float vmaxcomp(float3 v) {
	return max(v.x, max(v.y, v.z));
}

float3 vabs(float3 v) {
	return float3(
		abs(v.x),
		abs(v.y),
		abs(v.z)
	);
}

struct shape {
	short col=0x000F;

	virtual float signedDist(float3 pt) { return 0; }
};

struct sphere : shape {
	float3 pos;
	float rad;

	sphere(float3 pos_, float rad_, short col_) {
		pos=pos_;
		rad=rad_;
		col=col_;
	}

	float signedDist(float3 pt) override {
		float3 p=pt-pos;
		return length(p)-rad;
	}
};

struct box : shape {
	float3 pos, dim;

	box(float3 pos_, float3 dim_, short col_) {
		pos=pos_;
		dim=dim_;
		col=col_;
	}

	float signedDist(float3 pt) override {
		float3 p=pt-pos;
		float3 q=vabs(p)-dim;
		return length(vmax(q, float3()))+min(vmaxcomp(q), 0);
	}
};

struct boxFrame : shape {
	float3 pos, dim;
	float rad;

	boxFrame(float3 pos_, float3 dim_, float rad_, short col_) {
		pos=pos_;
		dim=dim_;
		rad=rad_;
		col=col_;
	}

	float signedDist(float3 pt) override {
		float3 p=pt-pos;
		p=vabs(p)-dim;
		float3 r(rad, rad, rad);
		float3 q=vabs(p+r)-r;
		return min(min(
			length(float3(max(p.x, 0), max(q.y, 0), max(q.z, 0)))+min(max(p.x, max(q.y, q.z)), 0),
			length(float3(max(q.x, 0), max(p.y, 0), max(q.z, 0)))+min(max(q.x, max(p.y, q.z)), 0)),
			length(float3(max(q.x, 0), max(q.y, 0), max(p.z, 0)))+min(max(q.x, max(q.y, p.z)), 0)
		);
	}
};

struct tri : shape {
	float3 a, b, c;

	tri(float3 a_, float3 b_, float3 c_, short col_) {
		a=a_;
		b=b_;
		c=c_;
		col=col_;
	}

	float signedDist(float3 p) override {
		float3 ba=b-a; float3 pa=p-a;
		float3 cb=c-b; float3 pb=p-b;
		float3 ac=a-c; float3 pc=p-c;
		float3 nor=cross(ba, ac);

		return sqrt(
			(sign(dot(cross(ba, nor), pa))+
				sign(dot(cross(cb, nor), pb))+
				sign(dot(cross(ac, nor), pc))<2)
			?
			min(min(
				dot2(ba*Maths::clamp(dot(ba, pa)/dot2(ba), 0, 1)-pa),
				dot2(cb*Maths::clamp(dot(cb, pb)/dot2(cb), 0, 1)-pb)),
				dot2(ac*Maths::clamp(dot(ac, pc)/dot2(ac), 0, 1)-pc))
			:
			dot(nor, pa)*dot(nor, pa)/dot2(nor)
		);
	}
};

class Demo : public Engine {
	public:
	float3 camPos, sunPos;
	float FOV;
	float camYaw, camPitch;
	float maxDist=100;
	std::vector<shape*> shapes;
	float glowThresh=0.5f;

	const char* asciiArr=" .,~=#&@";

	void setup() override {
		//initialize other stuff
		camPos=float3(0, 0, -5);
		sunPos=float3(0, 5, 0);

		FOV=Maths::PI/2;
		camYaw=Maths::PI/2;
		camPitch=Maths::PI/2;

		shapes.push_back(new sphere(float3(2.5f, 0, 0), 1, Raster::DARK_RED));

		shapes.push_back(new box(float3(-2.5f, 0, 0), float3(1, 1, 1), Raster::GREEN));

		shapes.push_back(new tri(float3(1, 1, 0), float3(-1, 1, 0), float3(0, -1, 0), Raster::CYAN));

		shapes.push_back(new boxFrame(float3(5, 0, 0), float3(1, 1, 1), 0.1f, Raster::DARK_YELLOW));
	}

	void update(float dt) override {
		//movement
		//move up down
		float speed=4.67f;
		if (getKey(VK_SPACE)) camPos.y+=speed*dt;
		if (getKey(VK_SHIFT)) camPos.y-=speed*dt;

		//move forward back
		float3 fbDir(cosf(camYaw), 0, sinf(camYaw));
		if (getKey('W')) camPos+=fbDir*speed*dt;
		if (getKey('S')) camPos-=fbDir*speed*dt;

		//move left right
		float3 lrDir(cosf(camYaw+Maths::PI/2), 0, sinf(camYaw+Maths::PI/2));
		if (getKey('A')) camPos+=lrDir*speed*dt;
		if (getKey('D')) camPos-=lrDir*speed*dt;

		//change view dir
		//look up down
		if (getKey(VK_UP)) camPitch-=dt;
		if (getKey(VK_DOWN)) camPitch+=dt;

		//look left right
		if (getKey(VK_LEFT)) camYaw+=dt;
		if (getKey(VK_RIGHT)) camYaw-=dt;

		//set sunpos if enter pressed
		if (getKey(VK_RETURN)) sunPos=camPos;

		camPitch=Maths::clamp(camPitch, EPSILON, Maths::PI-EPSILON);
		setTitle("RayMarching 3D @ "+std::to_string((int)framesPerSecond)+"fps");
	}

	void draw(Raster& rst) override {
		//background
		rst.setChar(' ');
		rst.fillRect(0, 0, width, height);

		//https://en.wikipedia.org/wiki/Ray_tracing_(graphics)
		//calc viewport
		float3 vUp(0, 1, 0);
		float3 camDir=float3(//cam dir [3d polar to cartesian]
			cosf(camYaw)*sinf(camPitch),
			cosf(camPitch),
			sinf(camYaw)*sinf(camPitch)
		);
		float3 b=cross(vUp, camDir);
		float3 tn=normalize(camDir);
		float3 bn=normalize(b);
		float3 vn=cross(tn, bn);

		//calc viewport sizes
		float gx=tanf(FOV/2);
		float gy=gx*(height-1)/(width-1);

		//stepping vectors
		float3 qx=bn*2*gx/(width-1);
		float3 qy=vn*2*gy/(height-1);
		float3 p1m=tn-bn*gx-vn*gy;

		bool* hitGrid=new bool[width*height];
		for (int i=0, x=0; i<width; i++, x++) {
			//y flipped
			for (int j=0, y=height-1; j<height; j++, y--) {
				//get dir and starting pos
				float3 pij=p1m+qx*i+qy*j;
				float3 checkPt=camPos;
				float3 dir=normalize(pij);

				//as long as we havent marched too far
				float totalDist=0;
				float closeDist=INFINITY;
				short colToUse=Raster::WHITE;
				bool hit=false;
				while (totalDist<maxDist) {
					//sort all shapes
					float sceneDist=INFINITY;
					for (shape* sptr:shapes) {
						shape& s=*sptr;
						float sDist=s.signedDist(checkPt);
						if (sDist<sceneDist) {
							sceneDist=sDist;
							colToUse=s.col;
						}
					}
					//save closest dist for glow
					closeDist=min(closeDist, sceneDist);
					//hit something, exit
					if (sceneDist<EPSILON) { hit=true; break; }
					//else march
					totalDist+=sceneDist;
					checkPt+=dir*sceneDist;
				}

				//hit something
				rst.setColor(colToUse);
				if (hit) rst.setChar(0x2588);
				else {
					rst.setChar(' ');

					//use glow
					if (closeDist<glowThresh) {
						float pct=Maths::map(closeDist, 0, glowThresh, 1, 0);
						int asi=Maths::clamp(pct*8, 0, 7);
						rst.setColor(Raster::WHITE);
						rst.setChar(asciiArr[asi]);
					}
				}
				rst.putPixel(x, y);
			}
		}

		//show fps
		rst.setChar(' ');
		rst.fillRect(0, 0, 16, 4);
		rst.setColor(Raster::WHITE);
		rst.drawString(0, 0, "yaw: "+std::to_string(camYaw));
		rst.drawString(0, 1, "pitch: "+std::to_string(camPitch));
	}
};

int main() {
	//init custom graphics engine
	Demo d=Demo();
	d.startWindowed(6, 160, 90);

	return 0;
}