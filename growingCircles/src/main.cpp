#include "Engine.h"
#include "maths/Maths.h"
#include "maths/vector/V2D.h"
#include "geom/AABB2D.h"
#include <vector>
#include <time.h>
using namespace displib;

struct circle {
	V2D pos;
	float rad;
	bool growing=true;

	AABB2D getAABB() {
		return AABB2D(pos.x-rad, pos.y-rad, pos.x+rad, pos.y+rad);
	}

	bool overlapCircle(circle other) {
		if (this->getAABB().overlapAABB(other.getAABB())) {//broad phase opt
			V2D sub=this->pos-other.pos;
			float mag=sub.mag();
			if (mag<this->rad+other.rad) return true;
		}
		return false;
	}

	bool containsPt(V2D pt) {
		if (this->getAABB().containsPt(pt)) {
			V2D sub=this->pos-pt;
			float mag=sub.mag();
			if (mag<this->rad) return true;
		}
		return false;
	}
};

class Demo : public Engine {
	public:
	int max_tries=80;
	float timer=0.0f;
	bool addingCircles=true;
	std::vector<circle> circles;

	void setup() override {}

	void update(float dt) override {
		if (addingCircles&&timer>0.2f) {//try to add circle this often
			timer=0.0f;
			bool tryFor=true;
			int tries=0;
			while (tries<max_tries) {
				V2D pt=V2D(//random pt
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
				addingCircles=false;//basically never spawn more circles
			}
		}


		//check all against all
		for (int i=0; i<circles.size(); i++) {
			for (int j=i+1; j<circles.size(); j++) {
				circle& a=circles.at(i);//make sure to be a ref.
				circle& b=circles.at(j);
				if (a.overlapCircle(b)) {
					a.growing=false;//stop both
					b.growing=false;
				}
			}
		}

		for (auto& c:circles) {
			if(c.growing)c.rad+=dt;//grow if we can
		}

		timer+=dt;
	}

	void draw(Raster& rst) override {
		rst.setChar(' ');
		rst.fillRect(0, 0, width, height);

		rst.setChar('#');
		for (auto& c:circles) {
			//color green if growing, red else
			rst.setColor(c.growing?Raster::GREEN:Raster::RED);
			rst.drawCircle(c.pos.x, c.pos.y, c.rad);
		}

		rst.setChar(' ');
		rst.fillRect(0, 0, 12, 2);
		rst.setColor(Raster::WHITE);
		rst.drawString(0, 0, "FPS: "+std::to_string((int)fps));
	}
};

int main() {
	srand(time(NULL));

	Demo d;
	d.start(8, 8, true);

	return 0;
}