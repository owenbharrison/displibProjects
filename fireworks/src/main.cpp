#include <time.h>
#include <vector>

#include "Engine.h"
#include "maths/Maths.h"
using namespace displib;

struct Particle {
	float2 pos, vel, acc;
	float age=0, lifeSpan;

	Particle(float2 pos_, float2 vel_, float lifeSpan_) {
		pos=pos_;
		vel=vel_;
		lifeSpan=lifeSpan_;
	}

	void update(float dt) {
		//euler explicit
		vel+=acc*dt;
		pos+=vel*dt;
		acc*=0;

		//age with change in time
		age+=dt;
	}

	void applyForce(float2 f) {
		acc+=f;
	}

	//too "old"
	bool isDead() {
		return age>lifeSpan;
	}

	void render(Raster& rst) {
		//ramp to show how "young" or "vibrant"
		float pct=Maths::map(age, 0, lifeSpan, 1, 0);
		int asi=Maths::clamp(pct*8, 0, 7);
		rst.setChar(" .,~=#&@"[asi]);
		rst.putPixel(pos);
	}
};

struct Firework {
	float2 pos, vel, acc=float2();
	short col;
	int numPtc;
	float maxRad;
	float minLifeSpan, maxLifeSpan;
	std::vector<Particle> particles;
	bool blown=false;

	Firework(float2 pos_, float2 vel_, short col_, int numPtc_, float maxRad_, float minLifeSpan_, float maxLifeSpan_) {
		pos=pos_;
		vel=vel_;
		col=col_;
		numPtc=numPtc_;
		maxRad=maxRad_;
		minLifeSpan=minLifeSpan_;
		maxLifeSpan=maxLifeSpan_;
	}

	void update(float dt) {
		//euler explicit
		vel+=acc*dt;
		pos+=vel*dt;
		acc*=0;

		//update all particles
		for (int i=particles.size()-1; i>=0; i--) {
			Particle& p=particles.at(i);

			p.update(dt);

			//"dynamically" clear particles if too "old"
			if (p.isDead()) {
				particles.erase(particles.begin()+i);
			}
		}
	}

	void applyForce(float2 f) {
		acc+=f;

		//apply force to all particles
		for (Particle& p:particles) {
			p.applyForce(f);
		}
	}

	//if height has reached max: dx/dt~0
	bool isAtApex() {
		return abs(vel.y)<1;
	}

	bool isDead() {
		return blown&&particles.size()==0;
	}

	void blowUp() {
		//only blow if we havent
		if (!blown) {
			for (int i=0; i<numPtc; i++) {
				//pointing in a random dir, random force, random final age
				float angle=Maths::random(-Maths::PI, Maths::PI);
				float rad=Maths::random(maxRad);
				float lifeSpan=Maths::random(minLifeSpan, maxLifeSpan);
				particles.push_back(Particle(pos, float2(cosf(angle), sinf(angle))*rad, lifeSpan));
			}

			blown=true;
		}
	}

	void render(Raster& rst) {
		//only show if we havent blown up
		if (!blown) {
			rst.putPixel(pos); rst.setColor(col);
		}

		//but always show all particles
		for (Particle& p:particles) p.render(rst);
	}
};

class Demo : public Engine {
	public:
	float2 grav;
	std::vector<Firework> fireworks;
	float timer=0;
	float nextFWTime;

	void setup() override {
		grav=float2(0, 9.8f);
		nextFWTime=Maths::random(0.4f, 1.1f);
	}

	void update(float dt) override {
		//update all fireworks
		for (int i=fireworks.size()-1; i>=0; i--) {
			Firework& f=fireworks.at(i);

			//shoot out particles!
			if (f.isAtApex()) {
				f.blowUp();
			}

			f.applyForce(grav);
			f.update(dt);

			//"dynamically" clear fireworks
			if (f.isDead()) {
				fireworks.erase(fireworks.begin()+i);
			}
		}

		//every so often
		if (timer>nextFWTime) {
			//reset timer
			timer=0;

			//choose random amount of time to spawn next one
			nextFWTime=Maths::random(0.4f, 1.1f);

			float x=Maths::random(width*0.1f, width*0.9f);
			float xv=Maths::random(-6, 6);
			float yv=-Maths::random(18, 42);
			int num=Maths::random(120, 350);
			short col=Maths::random(16);
			fireworks.push_back(Firework(float2(x, height), float2(xv, yv), col, num, width/12, 1.3f, 2.7f));
		}

		//always update timer
		timer+=dt;
	}

	void draw(Raster& rst) override {
		//background
		rst.setChar(' ');
		rst.fillRect(0, 0, width, height);

		//show all fireworks
		int total=0;
		for (Firework& f:fireworks) {
			rst.setChar('#');
			rst.setColor(f.col);
			f.render(rst);
			total+=f.particles.size();
		}

		//show fps
		rst.setChar(' ');
		rst.fillRect(0, 0, 10, 2);
		rst.setColor(Raster::WHITE);
		rst.drawString(0, 0, "FPS: "+std::to_string((int)framesPerSecond));
		rst.drawString(0, 1, "particles: "+std::to_string(total));
	}
};

int main() {
	srand(time(NULL));

	//init custom graphics engine
	Demo d=Demo();
	d.startFullscreen(8);

	return 0;
}