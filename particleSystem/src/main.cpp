#include <time.h>
#include <vector>

#include "Engine.h"
#include "maths/Maths.h"
using namespace displib;

struct Particle {
	V2D pos, vel, acc=V2D();
	float age=0, lifeSpan, rad, rot=0, rotSpeed;
	int sides;

	Particle(V2D pos_, V2D vel_, float rad_, int sides_, float lifeSpan_, float rotSpeed_) {
		pos=pos_;
		vel=vel_;
		rad=rad_;
		sides=sides_;
		lifeSpan=lifeSpan_;
		rotSpeed=rotSpeed_;
	}

	void update(float dt) {
		//euler explicit
		vel+=acc*dt;
		pos+=vel*dt;
		acc*=0;

		//age with change in time
		age+=dt;
		rot+=rotSpeed*dt;
	}

	void applyForce(V2D f) {
		acc+=f;
	}

	//too "old"
	bool isDead() {
		return age>lifeSpan;
	}

	void show(Raster& rst) {
		//ramp to show how "young" or "vibrant"
		float pct=Maths::map(age, 0, lifeSpan, 1, 0);
		int asi=Maths::clamp(pct*8, 0, 7);
		rst.setChar(" .,~=#&@"[asi]);
		for (int i=0; i<sides; i++) {
			float a0=Maths::map(i, 0, sides, 0, Maths::TAU)+rot;
			float a1=Maths::map((i+1)%sides, 0, sides, 0, Maths::TAU)+rot;
			V2D v0=V2D::fromAngle(a0)*rad+pos;
			V2D v1=V2D::fromAngle(a1)*rad+pos;
			rst.drawLine(v0.x, v0.y, v1.x, v1.y);
		}
	}
};

class Demo : public Engine {
	public:
	V2D grav;
	float timer=0;
	std::vector<Particle> particles;

	void setup() override {
		grav=V2D(0, 32);
	}

	void update(float dt) override {
		for (int i=0; i<particles.size();i++) {
			Particle& p=particles.at(i);

			p.applyForce(grav);

			p.update(dt);

			if (p.isDead()) {
				particles.erase(particles.begin()+i);
				i--;
			}
		}

		if (timer>0.05) {
			timer=0;

			if (getKey(VK_SPACE)) {
				float angle=Maths::random(-Maths::PI, Maths::PI);
				float speed=Maths::random(17, 35);
				V2D vel=V2D::fromAngle(angle)*speed;
				float rad=Maths::random(3, 6);
				int sides=Maths::random(3, 6);//tri, quad, or penta
				float lifeSpan=Maths::random(3, 6); 
				float rotSpeed=Maths::random(2, 6)*(Maths::random()>0.5f?-1:1);
				particles.push_back(Particle(V2D(mouseX, mouseY), vel, rad, sides, lifeSpan, rotSpeed));
			}
		}
		timer+=dt;
	}

	void draw(Raster& rst) override {
		rst.setChar(' ');
		rst.fillRect(0, 0, width, height);

		for (Particle& p:particles) {
			p.show(rst);
		}

		setTitle("Simple Particle System @ "+std::to_string((int)framesPerSecond)+"fps");
	}
};

int main() {
	srand(time(NULL));

	//init custom graphics engine
	Demo d=Demo();
	d.startWindowed(3, 320, 180);

	return 0;
}