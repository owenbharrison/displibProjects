#include <vector>
#include <time.h>

#include "Engine.h"
#include <geom/AABB2D.h>
#include "maths/Maths.h"
using namespace displib;

struct ptc {
	float2 pos, oldpos, vel, acc;
	float rad;

	ptc(float2 pos, float rad) : pos(pos), rad(rad){}

	void update(float dt) {
		oldpos=pos;
		vel+=acc*dt;
		pos+=vel*dt;
		acc*=0;
	}

	void applyForce(float2 f) {
		acc+=f;
	}

	AABB2D getAABB() {
		return AABB2D(
			float2(pos.x-rad, pos.y-rad),
			float2(pos.x+rad, pos.y+rad)
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
		restLen=length(getA().pos-getB().pos);
		stiff=stiff_;
		damp=damp_;
	}

	ptc& getA() { return *a; }
	ptc& getB() { return *b; }

	void update() {
		float2 sub=getB().pos-getA().pos;
		float2 dir=normalize(sub);
		float fs=stiff*(length(sub)-restLen);
		float fd=dot(dir, getB().vel-getA().vel)*damp;
		float2 f=dir*(fs+fd);

		getA().applyForce(f);
		getB().applyForce(f*-1);
	}

	void show(Raster& rst) {
		float2 a=getA().pos;
		float2 b=getB().pos;
		rst.drawLine(a, b);
	}
};

struct barrier {
	float2 a, b;
	float rad;

	void render(Raster& rst) {
		rst.drawCircle(a, rad);
		rst.drawCircle(b, rad);
		float2 tang=normalize(b-a);
		float2 norm(-tang.y, tang.x);
		rst.drawLine(a+norm*rad, b+norm*rad);
		rst.drawLine(a-norm*rad, b-norm*rad);
	}
};

class Demo : public Engine {
	public:
	float2 grav;
	std::vector<ptc> ptcs;
	std::vector<barrier> barriers;
	std::vector<std::pair<float2, float2>> connections;
	float timer=0;
	float stiff, damp;

	float2* heldVec=nullptr;
	bool held=false, wasHeld=false;

	bool showBlob=true;
	bool toggleKey=false, wasToggleKey=false;

	bool showConnections=false;

	const char* asciiArr=" .,~=#&@";
	short* colorArr=new short[8]{
		Raster::DARK_BLUE,
		Raster::BLUE,
		Raster::DARK_CYAN,
		Raster::CYAN,
		Raster::GREEN,
		Raster::DARK_YELLOW,
		Raster::RED,
		Raster::DARK_RED
	};

	void setup() override {
		grav=float2(0, 32);

		//top left ramp
		barriers.push_back({float2(0, 0), float2(width*0.1f, height*0.3f), 2});
		barriers.push_back({float2(width*0.1f, height*0.3f), float2(width*0.45f, height*0.45f), 2});

		//right to bottom ramp
		barriers.push_back({float2(width, 0), float2(width*0.875f, height*0.5f), 2});
		barriers.push_back({float2(width*0.875f, height*0.5f), float2(width*0.6f, height*0.8f), 2});
		barriers.push_back({float2(width*0.6f, height*0.8f), float2(width*0.4f, height*0.85f), 2});

		stiff=240.47f;
		damp=6.23f;
	}

	float lastDeltaTime=0;
	void update(float dt) override {
		showConnections=getKey('C');
		if (showConnections) connections.clear();

		//check mouse for barrier movement
		float2 mousePos(mouseX, mouseY);
		held=getKey(VK_SPACE);
		//on press
		if (held&&!wasHeld) {
			for (barrier& b:barriers) {
				if (length(b.a-mousePos)<b.rad) heldVec=&b.a;
				if (length(b.b-mousePos)<b.rad) heldVec=&b.b;
			}
		}
		//on release
		else if (!held&&wasHeld) heldVec=nullptr;
		wasHeld=held;

		//actually change the pos
		if (heldVec!=nullptr) *heldVec=mousePos;

		//for each particle
		for (int i=0; i<ptcs.size(); i++) {
			ptc& a=ptcs.at(i);
			//check all of OTHER the particles AFTER it
			for (int j=i+1; j<ptcs.size(); j++) {
				ptc& b=ptcs.at(j);
				//if touching optimization [aabb overlap]
				if (a.getAABB().overlapAABB(b.getAABB())) {
					float totalRad=a.rad+b.rad;
					float dist=length(a.pos-b.pos);
					//if particles touch [circle overlap]
					if (dist<totalRad) {
						if (showConnections) connections.push_back({a.pos, b.pos});

						//make new temp spring
						spr s(a, b, stiff, damp);
						s.restLen=a.rad+b.rad;
						s.update();
					}
				}
			}

			//check all barriers
			for (barrier& b:barriers) {
				float2 pa=a.pos-b.a, ba=b.b-b.a;
				float t=Maths::clamp(dot(pa, ba)/dot(ba, ba), 0, 1);
				float2 bPt=b.a+ba*t;
				if (length(bPt-a.pos)<b.rad+a.rad) {//"inside" barrier
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

			//in width?
			if (p.pos.x>0&&p.pos.x<width) {
				//too low? respawn.
				if (p.pos.y>height) p.pos.y=0;
			}
			else {
				p.vel.x=0;
				p.pos.x=width/2;
			}
		}

		//to start, spawn particles every so often
		if (timer>0.025f) {
			timer=0;
			if (ptcs.size()<370) {
				float x=Maths::random(width*0.3f, width*0.7f);
				ptcs.push_back(ptc(float2(x, height/5), Maths::random(2, 3)));
			}
		}
		timer+=dt;

		toggleKey=getKey(VK_RETURN);
		if (toggleKey&&!wasToggleKey) showBlob=!showBlob;
		wasToggleKey=toggleKey;

		setTitle("Particle Collisions w/ "+std::to_string(ptcs.size())+"ptcs @ "+std::to_string((int)framesPerSecond)+"fps");

		lastDeltaTime=dt;
	}

	void draw(Raster& rst) override {
		//background
		rst.setChar(' ');
		rst.fillRect(0, 0, width, height);

		//metaballs render
		if (showBlob) {
			rst.setColor(Raster::WHITE);
			bool* grid=new bool[width*height];
			for (int x=0; x<width; x++) {
				for (int y=0; y<height; y++) {
					//sum all "radius strengths"
					float sum=0;
					for (ptc& p:ptcs) {
						float2 sb=float2(x, y)-p.pos;
						float r=p.rad*1.48f;
						sum+=r*r/dot(sb, sb);
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
			for (ptc& p:ptcs) {
				//color based on speed
				float pct=length(p.vel)/6;
				int ci=Maths::clamp(pct, 0, 7);
				rst.setColor(colorArr[ci]);
				//inside
				rst.setChar('.');
				rst.fillCircle(p.pos, p.rad);
				//outline
				rst.setChar('P');
				rst.drawCircle(p.pos, p.rad);
			}
		}

		rst.setChar(0x2588);
		rst.setColor(Raster::WHITE);
		for (barrier& b:barriers) b.render(rst);

		if (showConnections) {
			rst.setColor(Raster::DARK_GREY);
			for (const auto& connection:connections) {
				rst.drawLine(connection.first, connection.second);
			}
		}
	}
};

int main() {
	srand(time(NULL));

	//init custom graphics engine
	Demo d;
	d.startWindowed(5, 220, 130);

	return 0;
}