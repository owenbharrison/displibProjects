#include "Engine.h"

#include "Softbody.h"
using namespace displib;

class Demo : public Engine {
	public:
	V2D grav;
	AABB2D bounds, mouseBox;
	Softbody jello, ball, flan, cloth;

	void setup() override {
		grav=V2D(0.0f, 21.0f);

		float stiff=240.47f;
		float damp=6.23f;
		jello.formRectangle(8, 8, AABB2D(width*0.1f, height*0.25f, width*0.3f, height*0.75f), stiff, damp);
		ball.formEllipse(4, 12, AABB2D(width*0.4f, height*0.4f, width*0.6f, height*0.7f), stiff*1.4, damp);
		flan.formRectangle(9, 10, AABB2D(width*0.7f, height*0.33f, width*0.9f, height*0.67f), stiff, damp);
		cloth.formCloth(24, 12, AABB2D(width*0.3f, 0, width*0.7f, height*0.4f), stiff, damp);

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
		flan.applyForce(grav/2);

		//add grav to all particles
		jello.applyForce(grav);
		ball.applyForce(grav);
		flan.applyForce(grav/2);
		cloth.applyForce(grav*2);

		//use euler explicit on all particles
		jello.update(dt);
		ball.update(dt);
		flan.update(dt);
		cloth.update(dt);

		//make sure we are in the screen, out of the mouseBox
		jello.constrainOut(mouseBox);
		jello.constrainIn(bounds);
		ball.constrainOut(mouseBox);
		ball.constrainIn(bounds);
		flan.constrainOut(mouseBox);
		flan.constrainIn(bounds);
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
		jello.renderSprings(rst);
		rst.setChar('b');
		ball.renderSprings(rst);
		rst.setChar('f');
		flan.renderSprings(rst);
		rst.setChar('c');
		cloth.renderSprings(rst);

		rst.setChar('*');
		jello.renderParticles(rst);
		rst.setChar('@');
		ball.renderParticles(rst);
		rst.setChar('#');
		flan.renderParticles(rst);

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
	d.start(8, 8, true);

	return 0;
}
