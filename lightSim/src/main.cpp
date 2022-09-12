#include <vector>
#include <time.h>

#include "Engine.h"
#include "maths/Maths.h"
using namespace displib;

struct Ray {
	float2 origin, dir;
};

struct LineSegment {
	float2 a, b;
};

struct Mirror : LineSegment {
	Mirror(float2 a_, float2 b_) {
		a=a_;
		b=b_;
	}

	float2 getNorm() {
		float2 tang=normalize(a-b);
		return float2(-tang.y, tang.x);
	}

	void render(Raster& rst) {
		rst.drawLine(a, b);
	}
};

//https://math.stackexchange.com/a/13263
float2 reflectVec(float2 inc, float2 norm) {
	return inc-norm*(2*dot(inc, norm));//r=d-2(d.n)n
}

//https://stackoverflow.com/a/29758766
float2 refractVec(float2 inc, float2 norm, float n1, float n2) {
	float r=n1/n2;
	float c=-dot(norm, inc);
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
		setTitle("Simple Light Bounce Sim");

		int num=6;
		//randomlt spawn mirrors
		for (int i=0; i<num; i++) {
			float x1=Maths::random(0, width);
			float y1=Maths::random(0, height);
			float x2=Maths::random(0, width);
			float y2=Maths::random(0, height);
			mirrors.push_back(Mirror(float2(x1, y1), float2(x2, y2)));
		}
		//make "max val" for last ray to travel
		maxLength=sqrtf(width*width+height*height);
	}

	void shutdown() {
		mirrors.clear();
	}

	void update(float dt) override {
		//change direction with keyboard
		if (getKey('A')) angle-=2*dt;
		if (getKey('D')) angle+=2*dt;
	}

	void draw(Raster& rst) override {
		//background
		rst.setChar(' ');
		rst.fillRect(0, 0, width, height);

		//draw all "mirrors"
		rst.setChar('m');
		rst.setColor(Raster::RED);
		for (Mirror& m:mirrors) m.render(rst);

		//start by making a ray from the mouse pos in a desired direction
		float2 rayStart(mouseX, mouseY);
		Ray ray={rayStart, float2(cosf(angle), sinf(angle))};
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
					float t, u;
					Maths::lineLineIntersection(m.a, m.b, ray.origin, ray.origin+ray.dir, &t, &u);
					float dist=u;
					//in mirror bounds and in correct dir
					if (t>0&&t<1&&dist>0) {
						//basically sort the lightsiom 
						if (dist<record) {
							record=dist;
							//save the sorted mirror
							chosenMirror=&m;
						}
					}
				}
			}

			//if viable:
			rst.setColor(Raster::WHITE);
			if (chosenMirror!=nullptr) {
				//use closest "mirror"
				Mirror& mirrorToUse=*chosenMirror;
				float2 ixPt=ray.origin+ray.dir*record;
				//draw line from original ray to intersect with "shading"
				//map along ascii ramp
				float fIndex=Maths::map(numBounces, 0, maxBounces-1, asciiLen-1, 0);
				int nIndex=Maths::clamp(fIndex, 0, asciiLen-1);
				rst.setChar(asciiArr[nIndex]);
				rst.drawLine(ray.origin, ixPt);
				//set "next" ray
				ray={ixPt, reflectVec(ray.dir, mirrorToUse.getNorm())};
				//save this used mirror so we dont self intersect
				lastMirror=chosenMirror;
				numBounces++;
			}
			else break;
		}
		if (numBounces<maxBounces) {
			float2 endPt=ray.origin+ray.dir*maxLength;
			rst.drawLine(ray.origin, endPt);
		}
	}
};

int main() {
	srand(time(NULL));

	//init custom graphics engine
	Demo d=Demo();
	d.startWindowed(8, 160, 90);

	return 0;
}