#include <iostream>
#include <time.h>
#include <vector>

#include "Engine.h"
#include "maths/Maths.h"
using namespace displib;

class Demo : public Engine {
	public:
	V2D p0, p1, p2, p3;
	std::vector<V2D> trail;
	float timer=0;

	V2D randomPt() {
		//random in screen
		return V2D(
			Maths::random(0, width),
			Maths::random(0, height)
		);
	}

	V2D lerp(V2D a, V2D b, float t) {
		//linear interpolate between two [x, y] objects
		return (b-a)*t+a;
	}

	void setup() override {
		//spread points randomly
		p0=randomPt();
		p1=randomPt();
		p2=randomPt();
		p3=randomPt();
		trail.clear();
	}

	void update(float dt) override {
		//every so often, reset
		if (timer>7.5) {
			setup();
			timer=0;
		}

		timer+=dt;
	}

	void draw(Raster& rst) override {
		//background
		rst.setChar(32);
		rst.fillRect(0, 0, width, height);

		//draw base lines
		rst.setChar('.');
		rst.drawLine(p0.x, p0.y, p1.x, p1.y);
		rst.drawLine(p1.x, p1.y, p2.x, p2.y);
		rst.drawLine(p2.x, p2.y, p3.x, p3.y);

		//draw first lerps
		float t=Maths::map(cosf(totalDt*0.67f), -1, 1, 0, 1);
		V2D lp0=lerp(p0, p1, t);
		V2D lp1=lerp(p1, p2, t);
		V2D lp2=lerp(p2, p3, t);
		rst.drawLine(lp0.x, lp0.y, lp1.x, lp1.y);
		rst.drawLine(lp1.x, lp1.y, lp2.x, lp2.y);

		//draw second lerp
		V2D llp0=lerp(lp0, lp1, t);
		V2D llp1=lerp(lp1, lp2, t);
		rst.drawLine(llp0.x, llp0.y, llp1.x, llp1.y);

		//draw "final" pt
		rst.setChar('@');
		V2D finalPt=lerp(llp0, llp1, t);
		int num=trail.size();
		if (num>1) {
			for (int i=1; i<num; i++) {
				V2D a=trail.at(i);
				V2D b=trail.at(i-1);
				rst.drawLine(a.x, a.y, b.x, b.y);
			}
		}
		trail.push_back(finalPt);
		if (num>fps*2) {
			trail.erase(trail.begin());
		}

		//draw pts
		rst.fillRect(p0.x-1, p0.y, 3, 3);
		rst.fillRect(p1.x-1, p1.y, 3, 3);
		rst.fillRect(p2.x-1, p2.y, 3, 3);
		rst.fillRect(p3.x-1, p3.y, 3, 3);

		//show fps
		rst.setChar(' ');
		rst.fillRect(0, 0, 10, 3);
		rst.drawString(0, 0, "FPS: "+std::to_string((int)fps));
	}
};

int main() {
	srand(time(NULL));

	//init custom graphics engine
	Demo d=Demo();
	d.start(10, 10, true);

	return 0;
}
