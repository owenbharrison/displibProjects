#include <vector>
#include <time.h>

#include "Engine.h"
#include <geom/AABB2D.h>
#include "maths/Maths.h"
using namespace displib;

struct ptc {
	V2D pos, oldpos, vel, acc;
	float rad;

	ptc(V2D pos_, float rad_) {
		pos=pos_;
		rad=rad_;
	}

	void update(float dt) {
		oldpos=pos;
		vel+=acc*dt;
		pos+=vel*dt;
		acc*=0;
	}

	void applyForce(V2D f) {
		acc+=f;
	}

	AABB2D getAABB() {
		return AABB2D(
			V2D(pos.x-rad, pos.y-rad),
			V2D(pos.x+rad, pos.y+rad)
		);
	}

	void checkAABB(AABB2D a) {
		//bounds detection and bounce
		if (pos.x<a.min.x+rad) {
			pos.x=a.min.x+rad;
			vel.x*=-1;
		}
		if (pos.y<a.min.y+rad) {
			pos.y=a.min.y+rad;
			vel.y*=-1;
		}
		if (pos.x>a.max.x-rad) {
			pos.x=a.max.x-rad;
			vel.x*=-1;
		}
		if (pos.y>a.max.y-rad) {
			pos.y=a.max.y-rad;
			vel.y*=-1;
		}
	}

	void render(Raster& rst) { rst.putPixel(pos.x, pos.y); }
};

struct spr {
	ptc* a=nullptr, * b=nullptr;
	float restLen=0, stiff=0, damp=0;

	spr() {}

	spr(ptc& a_, ptc& b_, float stiff_, float damp_) {
		a=&a_;
		b=&b_;
		restLen=(getA().pos-getB().pos).mag();
		stiff=stiff_;
		damp=damp_;
	}

	ptc& getA() { return *a; }
	ptc& getB() { return *b; }

	void update() {
		V2D sub=getB().pos-getA().pos;
		V2D dir=V2D::normal(sub);
		float fs=stiff*(sub.mag()-restLen);
		float fd=dir.dot(getB().vel-getA().vel)*damp;
		V2D f=dir*(fs+fd);

		getA().applyForce(f);
		getB().applyForce(f*-1);
	}

	void show(Raster& rst) {
		V2D a=getA().pos;
		V2D b=getB().pos;
		rst.drawLine(a, b);
	}
};

struct barrier {
	V2D a, b;
	float rad;

	void render(Raster& rst) {
		rst.drawCircle(a, rad);
		rst.drawCircle(b, rad);
		V2D tang=V2D::normal(b-a);
		V2D norm(-tang.y, tang.x);
		V2D ap=a+norm*rad;
		V2D an=a-norm*rad;
		V2D bn=b-norm*rad;
		V2D bp=b+norm*rad;
		rst.drawLine(ap, bp);
		rst.drawLine(an, bn);
	}
};

class Demo : public Engine {
	public:
	V2D grav;
	std::vector<ptc> ptcs;
	std::vector<barrier> barriers;
	float timer=0;
	float stiff, damp;
	const char* asciiArr=" .,~=#&@";
	bool keyDown=false;
	bool showBlob=true;

	int res, cols, rows;

	int ix(int i, int j) {
		return i+j*cols;
	}

	void setup() override {
		res=4;
		cols=width/res;
		rows=height/res;

		grav=V2D(0, 32);

		barriers.push_back({V2D(0, 0), V2D(0, height), 5});
		barriers.push_back({V2D(width, 0), V2D(width, height), 5});

		barriers.push_back({V2D(18, height/3), V2D(width/2+9, height/2), 5});
		barriers.push_back({V2D(width-18, height/2), V2D(width/2+9, height*2/3), 5});

		barriers.push_back({V2D(width/4, height-18), V2D(width/2-8, height), 5});

		stiff=240.47f;
		damp=6.23f;
	}

	void update(float dt) override {
		//move barriers
		V2D mp(mouseX, mouseY);
		if (getKey(VK_SPACE)) {
			for (barrier& b:barriers) {
				if ((b.a-mp).mag()<b.rad) b.a=mp;
				if ((b.b-mp).mag()<b.rad) b.b=mp;
			}
		}

		//for each particle
		for (int i=0; i<ptcs.size(); i++) {
			ptc& a=ptcs.at(i);
			//check all of the particles AFTER it
			for (int j=i+1; j<ptcs.size(); j++) {
				ptc& b=ptcs.at(j);
				//if touching optimization [aabb overlap]
				if (a.getAABB().overlapAABB(b.getAABB())) {
					float totalRad=a.rad+b.rad;
					float dist=(a.pos-b.pos).mag();
					//if particles touch [circle overlap]
					if (dist<totalRad) {
						//make new temp spring
						spr s(a, b, stiff, damp);
						s.restLen=a.rad+b.rad;
						s.update();
					}
				}
			}

			//check all barriers
			for (barrier& b:barriers) {
				V2D pa=a.pos-b.a, ba=b.b-b.a;
				float t=Maths::clamp(pa.dot(ba)/ba.dot(ba), 0, 1);
				V2D bPt=b.a+ba*t;
				if ((bPt-a.pos).mag()<b.rad+a.rad) {//"inside" barrier
					//make temp particle
					ptc p(bPt, b.rad);
					//make temp spring
					spr s(a, p, stiff, damp);
					s.restLen=a.rad+b.rad;
					//update spring like a particle collision
					s.update();
				}
			}
		}

		//update all particles
		for (ptc& p:ptcs) {
			p.applyForce(grav);

			//drag depends on vel [dv/dt=-cv]
			p.applyForce(p.vel*-0.33f);
			//euler explicit
			p.update(dt);

			if (p.pos.y>height) p.pos.y=0;
		}

		if (timer>0.03) {
			timer=0;
			if (ptcs.size()<330) {
				float x=Maths::random(width/5, width*4/5);
				ptcs.push_back(ptc(V2D(x, height/4), Maths::random(2, 3)));
			}
		}
		timer+=dt;

		bool switchKey=getKey(VK_RETURN);
		if (switchKey&&!keyDown) {
			keyDown=true;

			showBlob=!showBlob;
		}

		if (!switchKey&&keyDown) keyDown=false;
	}

	void draw(Raster& rst) override {
		//background
		rst.setChar(' ');
		rst.fillRect(0, 0, width, height);

		//metaballs
		if (showBlob) {
			rst.setChar('#');
			bool* grid=new bool[width*height];
			for (int x=0; x<width; x++) {
				for (int y=0; y<height; y++) {
					//sum all "radius strengths"
					float sum=0;
					for (ptc& p:ptcs) {
						V2D sb=V2D(x, y)-p.pos;
						float r=p.rad*1.5f;
						sum+=r*r/sb.dot(sb);
					}
					//make 0-1 float into ascii ramp value
					int asi=Maths::clamp(sum, 0, 7);
					rst.setChar(asciiArr[asi]);
					rst.putPixel(x, y);
				}
			}
			delete[] grid;
		}
		else {
			rst.setChar('p');
			for (ptc& p:ptcs) rst.drawCircle(p.pos, p.rad);
		}

		rst.setChar('b');
		for (barrier& b:barriers) b.render(rst);

		//show fps
		rst.setColor(Raster::WHITE);
		rst.drawString(0, 0, "FPS: "+std::to_string((int)framesPerSecond));
		rst.drawString(0, 1, "num: "+std::to_string(ptcs.size()));
	}
};

int main() {
	srand(time(NULL));

	//init custom graphics engine
	Demo d=Demo();
	d.startFullscreen(8);

	return 0;
}