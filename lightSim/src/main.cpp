#include <iostream>
#include <vector>
#include <time.h>

#include "Engine.h"
#include "maths/Maths.h"
using namespace displib;

struct Ray {
	V2D origin, dir;
};

struct LineSegment {
	V2D a, b;
};

struct Mirror : LineSegment {
	Mirror(V2D a_, V2D b_) {
		a=a_;
		b=b_;
	}

	V2D getNorm() {
		V2D tang=V2D::normal(a-b);
		return V2D(-tang.y, tang.x);
	}
};

//https://math.stackexchange.com/a/13263
V2D reflectVec(V2D inc, V2D norm) {
	return inc-norm*(2*inc.dot(norm));//r=d-2(d.n)n
}

//https://stackoverflow.com/a/29758766
V2D refractVec(V2D inc, V2D norm, float n1, float n2) {
	float r=n1/n2;
	float c=-norm.dot(inc);
	return inc*r+norm*(r*c-sqrt(1-r*r*(1-c*c)));
}

class Demo : public Engine {
	public:
	float angle=0;
	float maxLength;
	const int maxBounces=20;
	std::vector<Mirror> mirrors;
	const char* asciiArr=" .=+#@";
	const int asciiLen=strlen(asciiArr);

	void setup() override {
		int num=6;
		//randomlt spawn mirrors
		for (int i=0; i<num; i++) {
			float x1=Maths::random(0, width);
			float y1=Maths::random(0, height);
			float x2=Maths::random(0, width);
			float y2=Maths::random(0, height);
			mirrors.push_back(Mirror(V2D(x1, y1), V2D(x2, y2)));
		}
		//make "max val" for last ray to travel
		maxLength=sqrtf(width*width+height*height);
	}

	void update(float dt) override {
		//change direction with keyboard
		if (getKey('A')) angle-=2*dt;
		if (getKey('D')) angle+=2*dt;
	}

	void draw(Raster& rst) override {
		//background
		rst.setChar(32);
		rst.fillRect(0, 0, width, height);

		//draw all "mirrors"
		rst.setChar('m');
		for (Mirror& m:mirrors) {
			rst.drawLine(m.a.x, m.a.y, m.b.x, m.b.y);
		}

		//start by making a ray from the mouse pos in a desired direction
		V2D rayStart(mouseX, mouseY);
		Ray ray={rayStart, V2D::fromAngle(angle)};
		Mirror* lastMirror=nullptr;
		//make sure we dont get infinite bouncing
		int numBounces=0;
		while(numBounces<maxBounces){
			//check all mirrors
			Mirror* chosenMirror=nullptr;
			float record=INFINITY;
			for (Mirror& m:mirrors) {
				//make sure not to use the last mirror so we dont get "self" intersections, basically epsilon esc.
				if (&m!=lastMirror) {
					//ray line intersect
					float* tu=Maths::lineLineIntersection(m.a, m.b, ray.origin, ray.origin+ray.dir);
					float dist=tu[1];
					//in mirror bounds and in correct dir
					if (tu[0]>0&&tu[0]<1&&dist>0) {
						//basically sort the lightsiom 
						if (dist<record) {
							record=dist;
							//save the sorted mirror
							chosenMirror=&m;
						}
					}
				}
			}

			if (chosenMirror!=nullptr) {
				//use closest "mirror"
				Mirror& mirrorToUse=*chosenMirror;
				V2D ixPt=ray.origin+ray.dir*record;
				//draw line from original ray to intersect with "shading"
				//map along ascii ramp
				float fIndex=Maths::map(numBounces, 0, maxBounces-1, asciiLen-1, 0);
				int nIndex=Maths::clamp(fIndex, 0, asciiLen-1);
				rst.setChar(asciiArr[nIndex]);
				rst.drawLine(ray.origin.x, ray.origin.y, ixPt.x, ixPt.y);
				//set "next" ray
				ray={ixPt, reflectVec(ray.dir, mirrorToUse.getNorm())};
				//save this used mirror so we dont self intersect
				lastMirror=chosenMirror;
				numBounces++;
			}
			else break;
		}
		if (numBounces<maxBounces) {
			V2D endPt=ray.origin+ray.dir*maxLength;
			rst.drawLine(ray.origin.x, ray.origin.y, endPt.x, endPt.y);
		}

		//show fps
		rst.setChar(' ');
		rst.fillRect(0, 0, 10, 2);
		rst.drawString(0, 0, "FPS: "+std::to_string((int)fps));
	}
};

int main() {
	srand(time(NULL));

	//init custom graphics engine
	Demo d=Demo();
	d.start(6, 6, true);

	return 0;
}