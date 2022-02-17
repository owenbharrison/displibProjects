#include <iostream>
#include <time.h>

#include "Engine.h"
#include "maths/Maths.h"
#include "geom/Poly2D.h"
using namespace displib;

class Demo : public Engine {
	public:
	V2D ctr;
	Poly2D poly;

	void setup() override {
		ctr=V2D(width/2, height/2);
		int num=12;
		V2D* pts=new V2D[num];
		//make random poly (sort of like an asteroid)
		for (int i=0; i<num; i++) {
			float angle=Maths::map(i, 0, num, 0, Maths::TAU);
			float rad=Maths::random(8, 34);
			pts[i]=V2D::fromAngle(angle)*rad;
		}
		poly=Poly2D(ctr, pts, num);
	}

	void update(float dt) override {
		//put poly on mouse
		poly.pos=V2D(mouseX, mouseY);

		//spin it some
		poly.angle+=0.5f*dt;

		//update the poly
		poly.updatePoints();
	}

	void draw(Raster& rst) override {
		//background
		rst.setChar(32);
		rst.fillRect(0, 0, width, height);

		//show poly bounds
		rst.setChar(46);
		poly.getAABB().render(rst);

		//show poly
		rst.setChar(64);
		poly.render(rst);

		//show fps
		rst.setChar(32);
		rst.fillRect(0, 0, 10, 2);
		rst.drawString(0, 0, "FPS: "+std::to_string((int)fps));
	}
};

int main() {
	srand(time(NULL));

	//init custom graphics engine
	Demo d=Demo();
	d.start(12, 12, true);

	return 0;
}
