#include <time.h>
#include <vector>

#include "Engine.h"
#include "geom/AABB2D.h"
#include "maths/Maths.h"
using namespace displib;

struct Particle {
	float2 pos, vel, acc=float2();
	float age=0, lifeSpan, rad, rot=0, rotSpeed;
	int sides;

	Particle(float2 pos_, float2 vel_, float rad_, int sides_, float lifeSpan_, float rotSpeed_) {
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

	void applyForce(float2 f) {
		acc+=f;
	}

	AABB2D getAABB() {
		//sort to find extreme points
		float nx=INFINITY, ny=INFINITY, mx=-INFINITY, my=-INFINITY;
		for (int i=0; i<sides; i++) {
			float a0=Maths::map(i, 0, sides, 0, Maths::TAU)+rot;
			float2 v0=float2(cosf(a0), sinf(a0))*rad+pos;
			nx=min(nx, v0.x);
			ny=min(ny, v0.y);
			mx=max(mx, v0.x);
			my=max(my, v0.y);
		}
		return AABB2D(nx, ny, mx, my);
	}

	//if too "old"
	bool isDead() {
		return age>lifeSpan;
	}

	void render(Raster& rst) {
		//ramp to show how "young" or "vibrant"
		float pct=Maths::map(age, 0, lifeSpan, 1, 0);
		int asi=Maths::clamp(pct*8, 0, 7);
		rst.setChar(" .,~=#&@"[asi]);
		//polar to cartesian madness
		for (int i=0; i<sides; i++) {
			float a0=Maths::map(i, 0, sides, 0, Maths::TAU)+rot;
			float a1=Maths::map((i+1)%sides, 0, sides, 0, Maths::TAU)+rot;
			float2 v0=float2(cosf(a0), sinf(a0))*rad+pos;
			float2 v1=float2(cosf(a1), sinf(a1))*rad+pos;
			rst.drawLine(v0, v1);
		}
	}
};

class Demo : public Engine {
	public:
	float2 grav;
	float timer=0;
	bool keyDown=false;
	bool showBounds=false;
	std::vector<Particle> particles;

	void setup() override {
		grav=float2(0, 32);
	}

	void update(float dt) override {
		//check to see if we show bounds
		bool switchKey=getKey(VK_RETURN);
		if (switchKey&&!keyDown) {
			keyDown=true;
			showBounds=!showBounds;
		}
		if (!switchKey&&keyDown) keyDown=false;

		//update particles
		for (int i=particles.size()-1; i>=0; i--) {
			Particle& p=particles.at(i);

			p.applyForce(grav);

			p.update(dt);

			//"dynamically" remove "dead" particles
			if (p.isDead()) {
				particles.erase(particles.begin()+i);
			}
		}

		//every so often
		if (timer>0.05) {
			timer=0;

			//when holding space
			if (getKey(VK_SPACE)) {
				//spawn new random particle
				float angle=Maths::random(-Maths::PI, Maths::PI);
				float speed=Maths::random(17, 35);
				float2 vel=float2(cosf(angle), sinf(angle))*speed;
				float rad=Maths::random(4, 9);
				int sides=Maths::random(3, 7);//tri, quad, penta, or hexa
				float lifeSpan=Maths::random(3, 6);
				float rotSpeed=Maths::random(2, 6)*(Maths::random()>0.5f?-1:1);
				particles.push_back(Particle(float2(mouseX, mouseY), vel, rad, sides, lifeSpan, rotSpeed));
			}
		}
		timer+=dt;
	}

	void draw(Raster& rst) override {
		//show background
		rst.setChar(' ');
		rst.fillRect(0, 0, width, height);

		for (Particle& p:particles) {
			//show particle
			p.render(rst);

			if (showBounds) {
				//show bounds 
				p.getAABB().render(rst);
			}
		}

		//update title
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