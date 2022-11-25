#include <list>
#include <algorithm>

#include "Engine.h"
#include "geom/AABB2D.h"
using namespace displib;

#define POINT_SIZE 2.49f
#define SPRING_SIZE 3

#define MC 16.7
#define SS 4
#define CD 0.09

#define STIFF 240.45f
#define DAMP 4.32f

//thanks to pezzza for verlet stuff!
//https://www.youtube.com/watch?v=lS_qeBy3aQI&t=181s&ab_channel=Pezzza%27sWork
struct point {
	float2 pos, oldpos, acc;
	bool locked=false;

	point() {}

	point(float2 pos_) : pos(pos_), oldpos(pos_) {}

	void update(float dt) {
		float2 vel=pos-oldpos;
		//save pos
		oldpos=pos;
		//verlet integrate
		if(!locked) pos+=vel+acc*dt*dt;
		//reset acc
		acc*=0;
	}

	void accelerate(float2 f) {
		acc+=f;
	}

	void checkAABB(AABB2D a) {
		float2 vel=pos-oldpos;

		if (pos.x<a.min.x+POINT_SIZE) {
			pos.x=a.min.x+POINT_SIZE;
			oldpos.x=pos.x+vel.x;
		}
		if (pos.y<a.min.y+POINT_SIZE) {
			pos.y=a.min.y+POINT_SIZE;
			oldpos.y=pos.y+vel.y;
		}
		if (pos.x>a.max.x-POINT_SIZE) {
			pos.x=a.max.x-POINT_SIZE;
			oldpos.x=pos.x+vel.x;
		}
		if (pos.y>a.max.y-POINT_SIZE) {
			pos.y=a.max.y-POINT_SIZE;
			oldpos.y=pos.y+vel.y;
		}
	}
};

struct stick {
	point* a=nullptr, * b=nullptr;
	float restLen=0;

	stick() {}

	stick(point& a_, point& b_) : a(&a_), b(&b_) {
		restLen=length(a->pos-b->pos);
	}

	void update() {
		float2 axis=a->pos-b->pos;
		float dist=length(axis);
		float2 n=axis/dist;
		float delta=restLen-dist;
		if (!a->locked) a->pos+=n*delta*.5;
		if (!b->locked) b->pos-=n*delta*.5;
	}
};

struct spring {
	point* a=nullptr, * b=nullptr;
	float restLen=0, stiff=0, damp=0;

	spring() {}

	spring(point& a_, point& b_, float stiff_, float damp_) : a(&a_), b(&b_), stiff(stiff_), damp(damp_) {
		restLen=length(a->pos-b->pos);
	}

	void update() {
		float2 sub=b->pos-a->pos;
		float2 dir=normalize(sub);
		float fs=stiff*(length(sub)-restLen);
		float2 aVel=a->pos-a->oldpos;
		float2 bVel=b->pos-b->oldpos;
		float fd=dot(dir, bVel-aVel)*damp;

		if (!a->locked) a->accelerate(dir*(fs+fd));
		if (!b->locked) b->accelerate(dir*-(fs+fd));
	}
};

class Demo : public Engine {
	public:
	float2 grav;
	AABB2D bounds;

	std::list<point> points;
	std::list<stick> sticks;
	std::list<spring> springs;

	bool running=false;
	bool pausePlay=false, wasPausePlay=false;
	bool addNode=false, wasAddNode=false;
	bool lock=false, wasLock=false;
	bool stickConnect=false, wasStickConnect=false;
	point* stickConnectStart=nullptr;
	bool springConnect=false, wasSpringConnect=false;
	point* springConnectStart=nullptr;
	bool hold=false, wasHold=false;
	point* heldPoint=nullptr;

	void setup() override {
		grav=float2(0, 32);

		bounds=AABB2D(0, 0, width, height);
	}

	void update(float dt) override {
		float2 mousePos(mouseX, mouseY);

		//for pause/play
		pausePlay=getKey(VK_SPACE);
		if (pausePlay&&!wasPausePlay) {
			stickConnectStart=nullptr;
			springConnectStart=nullptr;
			heldPoint=nullptr;

			running=!running;
		}
		wasPausePlay=pausePlay;

		//for modifying
		stickConnect=getKey('C');
		bool stickConnectChanged=stickConnect!=wasStickConnect;
		if (stickConnectChanged) {
			if (stickConnect) {//on key down
				//clear connection.
				stickConnectStart=nullptr;
				for (point& p:points) {
					float dist=length(mousePos-p.pos);
					if (dist<POINT_SIZE) {
						stickConnectStart=&p;
					}
				}
			}
			else {//on key up
				point* connectEnd=nullptr;
				for (point& p:points) {
					float dist=length(mousePos-p.pos);
					if (dist<POINT_SIZE) {
						connectEnd=&p;
					}
				}
				//dont connect to nothing or self.
				if (stickConnectStart!=nullptr&&connectEnd!=nullptr&&connectEnd!=stickConnectStart) {
					sticks.push_back(stick(*stickConnectStart, *connectEnd));
				}
				//clear connection.
				stickConnectStart=nullptr;
			}
		}
		wasStickConnect=stickConnect;

		springConnect=getKey('S');
		bool springConnectChanged=springConnect!=wasSpringConnect;
		if (springConnectChanged) {
			if (springConnect) {//on key down
				//clear connection.
				springConnectStart=nullptr;
				for (point& p:points) {
					float dist=length(mousePos-p.pos);
					if (dist<POINT_SIZE) {
						springConnectStart=&p;
					}
				}
			}
			else {//on key up
				point* connectEnd=nullptr;
				for (point& p:points) {
					float dist=length(mousePos-p.pos);
					if (dist<POINT_SIZE) {
						connectEnd=&p;
					}
				}
				//dont connect to nothing or self.
				if (springConnectStart!=nullptr&&connectEnd!=nullptr&&connectEnd!=springConnectStart) {
					springs.push_back(spring(*springConnectStart, *connectEnd, STIFF, DAMP));
				}
				//clear connection.
				springConnectStart=nullptr;
			}
		}
		wasSpringConnect=springConnect;

		//to add node
		addNode=getKey('A');
		bool toAdd=addNode&&!wasAddNode;
		wasAddNode=addNode;
		bool toDelete=getKey('D');
		if (toAdd||toDelete||stickConnectChanged||springConnectChanged) {
			//stop if modify
			running=false;

			//add at mouse
			if (toAdd) {//on key down
				//make sure "far" from others
				bool farAway=true;
				for (point& p:points) {
					float dist=length(mousePos-p.pos);
					if (dist<POINT_SIZE*2) {
						farAway=false;
						break;
					}
				}
				if (farAway) points.push_back(point(mousePos));
			}
			//remove at mouse
			if (toDelete) {
				//for each point
				std::list<point>::iterator pit=points.begin();
				while (pit!=points.end()) {
					point& p=*pit;
					float dist=length(mousePos-p.pos);
					//if close to mouse
					if (dist<POINT_SIZE) {
						//for each stick
						std::list<stick>::iterator sti=sticks.begin();
						while (sti!=sticks.end()) {
							stick& s=*sti;
							//if stick has said point
							if (s.a==&p||s.b==&p) {
								//remove stick
								sti=sticks.erase(sti);
							}
							else sti++;
						}

						//for each spring
						std::list<spring>::iterator spi=springs.begin();
						while (spi!=springs.end()) {
							spring& s=*spi;
							//if spring has said point
							if (s.a==&p||s.b==&p) {
								//remove spring
								spi=springs.erase(spi);
							}
							else spi++;
						}

						//remove point
						pit=points.erase(pit);
					}
					else pit++;
				}
			}
		}

		//mouse interact
		hold=getKey(VK_MBUTTON);
		if (hold!=wasHold) {//on key change
			//reset
			heldPoint=nullptr;
			if (hold) {//on key down
				//find point to "hold"
				for (point& p:points) {
					float dist=length(mousePos-p.pos);
					if (dist<POINT_SIZE) {
						heldPoint=&p;
					}
				}
			}
		}
		wasHold=hold;

		//locking
		lock=getKey('L');
		if (lock&&!wasLock) {
			for (point& p:points) {
				float dist=length(mousePos-p.pos);
				if (dist<POINT_SIZE) {
					p.locked=!p.locked;
				}
			}
		}
		wasLock=lock;

		//simulation actual update
		if (running) {
			//update multiple for stability
			float subDt=dt/SS;
			for (int i=0; i<SS; i++) {
				for (stick& s:sticks) {
					s.update();
				}

				for (spring& s:springs) {
					s.update();
				}

				//mouse constraint
				if (heldPoint!=nullptr) {
					point& pt=*heldPoint;
					float2 sub=mousePos-pt.pos;
					pt.pos+=sub*dt*MC;
					pt.oldpos=pt.pos;
				}

				for (point& p:points) {
					p.accelerate(grav);
					//drag
					p.oldpos+=(p.pos-p.oldpos)*dt*CD;

					p.update(subDt);

					p.checkAABB(bounds);
				}
			}
		}
		//mouse constraint
		else if (heldPoint!=nullptr) {
			heldPoint->pos=mousePos;
			heldPoint->oldpos=mousePos;
			//check all sticks
			for (stick& s:sticks) {
				//does it have this point
				if (s.a==heldPoint||s.b==heldPoint) {
					//reset restLen
					s.restLen=length(s.a->pos-s.b->pos);
				}
			}
			//check all springs
			for (spring& s:springs) {
				//does it have this point
				if (s.a==heldPoint||s.b==heldPoint) {
					//reset restLen
					s.restLen=length(s.a->pos-s.b->pos);
				}
			}
		}

		//update title
		std::string fpsStr=std::to_string((int)framesPerSecond);
		std::string runStr=running?"running":"not running";
		setTitle("Physics Drawer ["+runStr+"] @ "+fpsStr+"fps");
	}

	void draw(Raster& rst) override {
		float2 mousePos(mouseX, mouseY);

		//background
		rst.setChar(' ');
		rst.fillRect(0, 0, width, height);

		//show edge highlight
		rst.setChar(0x2588);
		if (!running) {
			rst.setColor(Raster::DARK_RED);
			rst.drawRect(0, 0, width, height);
		}

		//show sticks
		rst.setColor(Raster::DARK_GREY);
		for (stick& s:sticks) {
			rst.drawLine(s.a->pos, s.b->pos);
		}

		//show springs
		rst.setColor(Raster::WHITE);
		for (spring& s:springs) {
			float2 ba=s.b->pos-s.a->pos;
			float len=length(ba);
			float2 tang=ba/len;
			float2 norm(-tang.y, tang.x);
			int sign=1;
			float2 lastPt=s.a->pos;
			for (float t=0; t<len;t+=SPRING_SIZE,sign*=-1) {
				float2 pt=s.a->pos+tang*t+norm*SPRING_SIZE*sign;
				rst.drawLine(lastPt, pt);
				lastPt=pt;
			}
			rst.drawLine(lastPt, s.b->pos);
		}

		//show points
		for (point& p:points) {
			rst.setColor(Raster::WHITE);
			rst.fillCircle(p.pos, POINT_SIZE);
			if (p.locked) {
				rst.setColor(Raster::DARK_RED);
				rst.drawCircle(p.pos, POINT_SIZE);
			}
		}

		//show hover point
		point* possible=nullptr;
		rst.setColor(Raster::WHITE);
		for (point& p:points) {
			float dist=length(mousePos-p.pos);
			if (dist<POINT_SIZE) {
				possible=&p;
			}
		}
		if (possible!=nullptr) {
			rst.setColor(Raster::GREEN);
			rst.drawCircle(possible->pos, POINT_SIZE);
		}

		//show held point
		if (heldPoint!=nullptr) {
			rst.setColor(Raster::CYAN);
			rst.drawCircle(heldPoint->pos, POINT_SIZE);
		}

		//show stick connect pt
		if (stickConnectStart!=nullptr) {
			float fract=totalDeltaTime-(int)totalDeltaTime;
			bool blink=fract>.5f;
			rst.setColor(blink?Raster::DARK_MAGENTA:Raster::DARK_BLUE);
			rst.drawCircle(stickConnectStart->pos, POINT_SIZE);
			rst.drawLine(stickConnectStart->pos, mousePos);
		}

		//show spring connect pt
		if (springConnectStart!=nullptr) {
			float fract=totalDeltaTime-(int)totalDeltaTime;
			bool blink=fract>.5f;
			rst.setColor(blink?Raster::DARK_GREEN:Raster::DARK_YELLOW);
			rst.drawCircle(springConnectStart->pos, POINT_SIZE);
			rst.drawLine(springConnectStart->pos, mousePos);
		}
	}
};

int main() {
	//init custom graphics engine
	Demo d;
	d.startWindowed(5, 240, 135);

	return 0;
}