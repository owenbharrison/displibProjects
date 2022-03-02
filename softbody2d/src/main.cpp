#include "Engine.h"

#include "Softbody.h"
using namespace displib;

class Demo : public Engine {
	public:
	V2D grav;
	AABB2D bounds, mouseBox;
	Softbody jello, ball, cloth;

	void setup() override {
		grav=V2D(0.0f, 21.0f);

		float stiff=240.47f;
		float damp=6.23f;
		jello.formRectangle(7, 7, AABB2D(width*0.1f, height*0.25f, width*0.3f, height*0.75f), stiff, damp);
		ball.formEllipse(3, 9, AABB2D(width*0.7f, height*0.4f, width*0.85f, height*0.7f), stiff*1.4, damp);
		cloth.formCloth(12, 6, AABB2D(width*0.3f, 0, width*0.7f, height*0.4f), stiff, damp);

		bounds=AABB2D(0, 0, width, height);
		updateMouseBox();
	}

	void updateMouseBox() {
		mouseBox=AABB2D(mouseX-10, mouseY-8, mouseX+8, mouseY+10);
	}

	void update(float dt) override {
		if (getKey(' ')) {
			updateMouseBox();
		}

		//add grav to all particles
		jello.applyForce(grav);
		ball.applyForce(grav);

		//add grav to all particles
		jello.applyForce(grav);
		ball.applyForce(grav);
		cloth.applyForce(grav*2);

		//use euler explicit on all particles
		jello.update(dt);
		ball.update(dt);
		cloth.update(dt);

		//make sure we are in the screen, out of the mouseBox
		jello.constrainOut(mouseBox);
		jello.constrainIn(bounds);
		ball.constrainOut(mouseBox);
		ball.constrainIn(bounds);
		//cloth doesnt need to be constrained in
		cloth.constrainOut(mouseBox);
	}

	void draw(Raster& rst) override {
		//background
		rst.setChar(' ');
		rst.fillRect(0, 0, width, height);

		rst.setChar('#');
		mouseBox.render(rst);

		//draw springs
		rst.setChar('j');
		rst.setColor(Raster::GREEN);
		jello.renderSprings(rst);

		rst.setChar('b');
		rst.setColor(Raster::RED);
		ball.renderSprings(rst);

		rst.setChar('c');
		rst.setColor(Raster::GREY);
		cloth.renderSprings(rst);

		//draw particles
		rst.setChar('*');
		jello.renderParticles(rst);
		rst.setChar('@');
		ball.renderParticles(rst);

		//show fps
		rst.setChar(' ');
		rst.fillRect(0, 0, 10, 2);
		rst.setColor(Raster::WHITE);
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
