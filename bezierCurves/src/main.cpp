#include <time.h>
#include <vector>

#include "Engine.h"
#include "maths/Maths.h"
using namespace displib;

class Demo : public Engine {
	public:
	float2 p0, p1, p2, p3;
	std::vector<float2> trail;
	float timer=0;

	float2 randomPt() {
		//random in screen
		return float2(
			Maths::random(0, width),
			Maths::random(0, height)
		);
	}

	float2 lerp(float2 a, float2 b, float t) {
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
		//every so often, reset the "sim".
		if (timer>7.5) {
			setup();
			timer=0;
		}

		timer+=dt;

		setTitle("Bezier Curves @ "+std::to_string((int)framesPerSecond)+"fps");
	}

	void draw(Raster& rst) override {
		//background
		rst.setChar(' ');
		rst.fillRect(0, 0, width, height);

		//draw base lines
		rst.setChar('.');
		rst.setColor(Raster::RED);
		rst.drawLine(p0, p1);
		rst.drawLine(p1, p2);
		rst.drawLine(p2, p3);

		//draw first lerps
		float t=Maths::map(cosf(totalDeltaTime*0.67f), -1, 1, 0, 1);
		float2 lp0=lerp(p0, p1, t);
		float2 lp1=lerp(p1, p2, t);
		float2 lp2=lerp(p2, p3, t);
		rst.setColor(Raster::YELLOW);
		rst.drawLine(lp0, lp1);
		rst.drawLine(lp1, lp2);

		//draw second lerp
		float2 llp0=lerp(lp0, lp1, t);
		float2 llp1=lerp(lp1, lp2, t);
		rst.setColor(Raster::GREEN);
		rst.drawLine(llp0, llp1);

		//draw trail
		rst.setChar('#');
		rst.setColor(Raster::CYAN);
		float2 finalPt=lerp(llp0, llp1, t);
		int num=trail.size();
		if (num>1) {
			for (int i=1; i<num; i++) {
				float2 a=trail.at(i);
				float2 b=trail.at(i-1);
				rst.drawLine(a, b);
			}
		}
		//update trail, make sure it is not too long
		trail.push_back(finalPt);
		if (num>framesPerSecond*2) {
			trail.erase(trail.begin());
		}

		//draw pts
		rst.setChar('@');
		rst.setColor(Raster::WHITE);
		rst.putPixel(p0);
		rst.putPixel(p1);
		rst.putPixel(p2);
		rst.putPixel(p3);
	}
};

int main() {
	srand(time(NULL));

	//init custom graphics engine
	Demo d=Demo();
	d.startWindowed(4, 200, 200);

	return 0;
}