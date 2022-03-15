#include <time.h>

#include "Engine.h"
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
		poly.angle=totalDeltaTime/2;

		//update the poly
		poly.updatePoints();
	}

	void draw(Raster& rst) override {
		//background
		rst.setChar(' ');
		rst.fillRect(0, 0, width, height);

		//show poly bounds
		rst.setChar('.');
		rst.setColor(Raster::CYAN);
		poly.getAABB().render(rst);

		//show poly
		rst.setChar('@');
		rst.setColor(Raster::MAGENTA);
		poly.render(rst);

		//show fps
		rst.setChar(' ');
		rst.fillRect(0, 0, 10, 2);
		rst.setColor(Raster::WHITE);
		rst.drawString(0, 0, "FPS: "+std::to_string((int)framesPerSecond));
	}
};

int main() {
	srand(time(NULL));

	//init custom graphics engine
	Demo d=Demo();
	d.start(12, 12, true);

	return 0;
}
