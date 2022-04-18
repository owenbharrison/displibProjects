#include <vector>
#include <time.h>

#include "Engine.h"
#include "io/Stopwatch.h"
#include "physics/Spring.h"
#include "maths/Maths.h"
using namespace displib;

class Demo : public Engine {
	public:
	V2D grav;
	AABB2D bounds;
	std::vector<Particle> particles;
	float timer=0;
	float stiff, damp;
	Stopwatch updateWatch, drawWatch;
	const char* asciiArr=" .,~=#&@";

	void setup() override {
		grav=V2D(0, 9.8f);

		bounds=AABB2D(0, 0, width, height);

		stiff=240.47f;
		damp=6.23f;
	}

	void update(float dt) override {
		updateWatch.start();
		//for each particle
		for (int i=0; i<particles.size(); i++) {
			//check all of the particles AFTER it
			for (int j=i+1; j<particles.size(); j++) {
				Particle& a=particles.at(i);
				Particle& b=particles.at(j);
				//if touching optimization [aabb overlap]
				if (a.getAABB().overlapAABB(b.getAABB())) {
					float totalRad=a.rad+b.rad;
					float dist=(a.pos-b.pos).mag();
					//if particles touch [circle overlap]
					if (dist<totalRad) {
						//make new temp spring
						Spring s=Spring(a, b, stiff, damp);
						s.restLen=a.rad+b.rad;
						s.update();
					}
				}
			}
		}

		//update all particles
		for (Particle& p:particles) {
			p.applyForce(grav);

			//drag depends on vel [dv/dt=-cv]
			p.applyForce(p.vel*-0.33f);
			//euler explicit
			p.update(dt);
			//keep in window
			p.checkAABB(bounds);
		}
		updateWatch.stop();

		if (timer>0.05) {
			timer=0;
			if (getKey(VK_SPACE)) {
				float rad=Maths::random(3, 7);
				particles.push_back(Particle(V2D(mouseX, mouseY), rad));
			}
		}
		timer+=dt;
	}

	void draw(Raster& rst) override {
		drawWatch.start();
		//background
		rst.setChar(' ');
		rst.fillRect(0, 0, width, height);

		//draw particles
		rst.setChar('p');
		for (Particle& p:particles) {
			rst.drawCircle(p.pos.x, p.pos.y, p.rad);
		}
		drawWatch.stop();

		//show fps
		rst.setColor(Raster::WHITE);
		rst.drawString(0, 0, "FPS: "+std::to_string((int)framesPerSecond));
		rst.drawString(0, 1, "updateTime: "+std::to_string(updateWatch.getNanoseconds())+"ns");
		rst.drawString(0, 2, "drawTime: "+std::to_string(drawWatch.getNanoseconds())+"ns");
	}
};

int main() {
	srand(time(NULL));

	//init custom graphics engine
	Demo d=Demo();
	d.start(8, 8, true);

	return 0;
}