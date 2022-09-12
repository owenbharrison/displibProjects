#include "Engine.h"
#include "maths/Maths.h"
#include "maths/vector/float2.h"
#include "geom/AABB2D.h"
#include <vector>
#include <time.h>
using namespace displib;

struct circle {
	float2 pos;
	float rad;
	bool growing=true;

	AABB2D getAABB() {
		return AABB2D(pos.x-rad, pos.y-rad, pos.x+rad, pos.y+rad);
	}

	bool overlapCircle(circle other) {
		if (getAABB().overlapAABB(other.getAABB())) {//broad phase opt
			float2 sub=pos-other.pos;
			float mag=length(sub);
			if (mag<rad+other.rad) return true;
		}
		return false;
	}

	bool containsPt(float2 pt) {
		if (getAABB().containsPt(pt)) {
			float2 sub=pos-pt;
			float mag=length(sub);
			if (mag<rad) return true;
		}
		return false;
	}

	void render(Raster& rst) {
		rst.drawCircle(pos, rad);
	}
};

class Demo : public Engine {
	public:
	int max_tries=80;
	float timer=0.0f;
	bool addingCircles=true;
	std::vector<circle> circles;

	void update(float dt) override {
		if (addingCircles&&timer>0.2f) {//try to add circle this often
			timer=0.0f;
			bool tryFor=true;
			int tries=0;
			while (tries<max_tries) {
				float2 pt=float2(//random pt
					Maths::random(0, width),
					Maths::random(0, height)
				);

				bool inside=false;
				for (auto& c:circles) {//check all so we dont
					if (c.containsPt(pt)) {//add a circle in another circle
						inside=true;
						break;//opt
					}
				}
				if (inside)tries++;
				else {
					circles.push_back({pt, 0.0f});
					break;
				}
				tries++;
			}
			if (tries==max_tries) {
				//basically never spawn more circles
				addingCircles=false;
			}
		}


		//check all against all
		for (int i=0; i<circles.size(); i++) {
			for (int j=i+1; j<circles.size(); j++) {
				//make sure to be a ref.
				circle& a=circles.at(i);
				circle& b=circles.at(j);
				if (a.overlapCircle(b)) {
					//stop both
					a.growing=false;
					b.growing=false;
				}
			}
		}

		for (auto& c:circles) {
			//grow if we can
			if (c.growing)c.rad+=dt;
		}

		timer+=dt;
	}

	void draw(Raster& rst) override {
		//clear background
		rst.setChar(' ');
		rst.fillRect(0, 0, width, height);

		rst.setChar('#');
		for (auto& c:circles) {
			//color green if growing, red else
			rst.setColor(c.growing?Raster::GREEN:Raster::RED);
			c.render(rst);
		}

		//show fps
		rst.setChar(' ');
		rst.fillRect(0, 0, 12, 2);
		rst.setColor(Raster::WHITE);
		rst.drawString(0, 0, "FPS: "+std::to_string((int)framesPerSecond));
	}
};

int main() {
	srand(time(NULL));

	Demo d;
	d.startFullscreen(8);

	return 0;
}