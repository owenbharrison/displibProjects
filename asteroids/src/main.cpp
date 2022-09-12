#include <time.h>
#include <vector>

#include "Engine.h"
#include "geom/AABB2D.h"
#include "maths/Maths.h"
using namespace displib;

float2 float2FromAngle(float angle) {
	return float2(cosf(angle), sinf(angle));
}

struct Particle {
	float2 pos, vel;
	float lifespan, age=0;

	void update(float dt) {
		pos+=vel*dt;
		age+=dt;
	}

	//is too old?
	bool isDead() {
		return age>lifespan;
	}

	void render(Raster& rst) {
		//ramp to show how "young" or "vibrant"
		float pct=age/lifespan;
		int asi=Maths::clamp(pct*8, 0, 7);
		rst.setChar("@&#=~,. "[asi]);
		rst.putPixel(pos);
	}
};

struct Bullet {
	float2 pos, vel;

	void update(float dt) {
		pos+=vel*dt;
	}

	//is out of bounds?
	bool isOffscreen(AABB2D a) {
		return !a.containsPt(pos);
	}

	void render(Raster& rst) {
		rst.fillRect(pos.x-1, pos.y-1, 3, 3);
	}
};

struct Asteroid {
	float2 pos, vel;
	float rad=0;
	float2* model=nullptr, * points=nullptr;
	int numPts=0;

	Asteroid() {}

	Asteroid(float2 pos_, float2 vel_, float rad_, int numPts_) {
		pos=pos_;
		vel=vel_;
		rad=rad_;
		numPts=numPts_;
		model=new float2[numPts];
		points=new float2[numPts];
		for (int i=0; i<numPts; i++) {
			float angle=((float)i/(float)numPts)*Maths::PI*2;
			model[i]=float2FromAngle(angle)*Maths::random(rad*2/3, rad);
		}
	}

	void update(float dt) {
		pos+=vel*dt;
		for (int i=0; i<numPts; i++) {
			points[i]=model[i]+pos;
		}
	}

	//toroidal space
	void checkAABB(AABB2D a) {
		if (pos.x<a.min.x) pos.x=a.max.x;
		if (pos.y<a.min.y) pos.y=a.max.y;
		if (pos.x>a.max.x) pos.x=a.min.x;
		if (pos.y>a.max.y) pos.y=a.min.y;
	}

	//can or should this be split more?
	bool split(Asteroid& a, Asteroid& b) {
		//if asteroid is too small or not enough pts, dont bother splitting it
		if (rad<10||numPts<7) return false;
		float spdA=Maths::random(0.75f, 1), spdB=Maths::random(0.75f, 1);
		float radA=Maths::random(0.5f, 0.8f), radB=Maths::random(0.5f, 0.8f);
		a=Asteroid(pos, float2(-vel.y, vel.x)*spdA, rad*radA, numPts-4);
		b=Asteroid(pos, float2(vel.y, -vel.x)*spdB, rad*radB, numPts-4);
		return true;
	}

	AABB2D getAABB() {
		//sort to find extreme points
		float nx=INFINITY, ny=INFINITY, mx=-INFINITY, my=-INFINITY;
		for (int i=0; i<numPts; i++) {
			float2 p=points[i];
			nx=min(nx, p.x);
			ny=min(ny, p.y);
			mx=max(mx, p.x);
			my=max(my, p.y);
		}
		return AABB2D(nx, ny, mx, my);
	}

	//is pt inside asteroid?
	bool containsPt(float2 pt) {
		//aabb optimization
		if (getAABB().containsPt(pt)) {
			int numIx=0;
			for (int i=0; i<numPts; i++) {
				float2 c=points[i];
				float2 n=points[(i+1)%numPts];
				float t, u;
				Maths::lineLineIntersection(pt, pt+float2(1, 0), c, n, &t, &u);
				//ray line intersect
				if (t>0&&u>0&&u<1) numIx++;
			}
			return numIx%2==1;
		}
		return false;
	}

	void render(Raster& rst) {
		for (int i=0; i<numPts; i++) {
			float2 a=points[i];
			float2 b=points[(i+1)%numPts];
			rst.drawLine(a, b);
		}
	}
};

struct Ship {
	float2 pos, vel, acc;
	float rad=0, rot=0;

	Ship() {}

	Ship(float2 pos_, float rad_) {
		pos=pos_;
		rad=rad_;
	}

	void update(float dt) {
		//drag
		applyForce(vel*-0.46f);
		vel+=acc*dt;
		pos+=vel*dt;
		acc*=0;
	}

	void applyForce(float2 f) {
		acc+=f;
	}

	//toroidal space
	void checkAABB(AABB2D a) {
		if (pos.x<a.min.x) pos.x=a.max.x;
		if (pos.y<a.min.y) pos.y=a.max.y;
		if (pos.x>a.max.x) pos.x=a.min.x;
		if (pos.y>a.max.y) pos.y=a.min.y;
	}

	Bullet getBullet() {
		return {pos+float2FromAngle(rot)*rad, float2FromAngle(rot)*63};
	}

	//move forward
	void boost(float f) {
		applyForce(float2FromAngle(rot)*f);
	}

	void turn(float f) {
		rot+=f;
	}

	//random particle at end of ship
	Particle emitParticle() {
		float randAngle=Maths::random(-Maths::PI/3, Maths::PI/3);
		float speed=Maths::random(3, 6);
		float lifespan=Maths::random(1.6f, 3.8f);
		float2 vel=float2FromAngle(rot+Maths::PI+randAngle)*speed;
		return {pos-float2FromAngle(rot+randAngle/2)*rad, vel, lifespan};
	}

	float2* outline() {
		float2 fw=float2FromAngle(rot)*rad;
		float2 up(-fw.y, fw.x);
		return new float2[3]{
			pos+fw*1.2f,
			pos-fw+up,
			pos-fw-up
		};
	}

	AABB2D getAABB() {
		float2* o=outline();
		float2 a=o[0], b=o[1], c=o[2];
		delete[] o;
		return AABB2D(
			min(a.x, min(b.x, c.x)),
			min(a.y, min(b.y, c.y)),
			max(a.x, max(b.x, c.x)),
			max(a.y, max(b.y, c.y))
		);
	}

	void render(Raster& rst) {
		float2* lns=outline();
		rst.drawTriangle(lns[0], lns[1], lns[2]);
		delete[] lns;
	}
};

class Demo : public Engine {
	public:
	float particleTimer=0, bulletTimer=0, scoreTimer=0, warningTimer=0, endTimer=0;
	Ship ship;
	AABB2D bounds;
	std::vector<Asteroid> asteroids;
	std::vector<Bullet> bullets;
	std::vector<Particle> particles;
	int score=0, stage=0;
	bool won=false, lost=false;
	int warningStage=0, endStage=0;
	bool keyDown=false, debugMode=false;

	float2 randomPtOnEdge(AABB2D a) {
		float pct=Maths::random();
		if (pct<0.25f) return float2(Maths::random(a.min.x, a.max.x), a.min.y);//top
		else if (pct<0.5f) return float2(Maths::random(a.min.x, a.max.x), a.max.y);//bottom
		else if (pct<0.75f) return float2(a.min.x, Maths::random(a.min.y, a.max.y));//left
		else return float2(a.max.x, Maths::random(a.min.y, a.max.y));//right
	}

	Asteroid randomAsteroid() {
		float angle=Maths::random(-Maths::PI, Maths::PI);
		float speed=Maths::random(15, 27);
		float rad=Maths::random(width/16, width/10);
		int numPts=Maths::random(20, 28);
		return Asteroid(randomPtOnEdge(bounds), float2FromAngle(angle)*speed, rad, numPts);
	}

	void randomParticle(float2 pos, Particle& p) {
		float randAngle=Maths::random(-Maths::PI, Maths::PI);
		float speed=Maths::random(1, 6);
		float lifespan=Maths::random(1.6f, 3.8f);
		float2 vel=float2FromAngle(randAngle)*speed;
		p={pos, vel, lifespan};
	}

	void setup() override {
		bounds=AABB2D(0, 0, width, height);

		ship=Ship(float2(width/2, height/2), 4);
	}

	void update(float dt) override {
		//whether or not to turn on debugMode
		bool switchKey=getKey('D');
		if (switchKey&&!keyDown) {
			keyDown=true;
			debugMode=!debugMode;
		}
		if (!switchKey&&keyDown) keyDown=false;

		//update particles
		for (int i=particles.size()-1; i>=0; i--) {
			Particle& p=particles.at(i);
			p.update(dt);

			//"dynamically" clear those too old
			if (p.isDead()) {
				particles.erase(particles.begin()+i);
			}
		}

		//update bullets
		for (int i=bullets.size()-1; i>=0; i--) {
			Bullet& b=bullets.at(i);
			b.update(dt);

			//"dynamically" clear those offscreen
			if (b.isOffscreen(bounds)) {
				bullets.erase(bullets.begin()+i);
			}
		}

		//if "alive"
		bool boostKey=getKey(VK_UP);
		if (!lost) {
			//ship movement
			if (boostKey) ship.boost(87.43f);
			float turnSpeed=2.78f;
			if (getKey(VK_RIGHT)) ship.turn(turnSpeed*dt);
			if (getKey(VK_LEFT))  ship.turn(-turnSpeed*dt);

			//ship update
			ship.update(dt);
			ship.checkAABB(bounds);
		}

		//update asteroids
		float2* shipOutline=ship.outline();
		for (int i=asteroids.size()-1; i>=0; i--) {
			Asteroid& a=asteroids.at(i);
			a.update(dt);
			//when hit edge spawn on other side
			a.checkAABB(bounds);

			//if "alive"
			if (!won&&!lost) {
				bool shipHit=false;
				//check asteroid against all ship lines
				for (int j=0; j<3; j++) {
					float2 sp0=shipOutline[j];
					float2 sp1=shipOutline[(j+1)%3];
					//to all asteroid lines
					for (int k=0; k<a.numPts; k++) {
						float2 ap0=a.points[k];
						float2 ap1=a.points[(k+1)%a.numPts];
						float t, u;
						Maths::lineLineIntersection(sp0, sp1, ap0, ap1, &t, &u);
						//if even one hits, game over
						if (t>0&&t<1&&u>0&&u<1) shipHit=true;
					}
				}
				if (shipHit) {//end game
					lost=true;
					int numRand=Maths::random(56, 84);
					for (int i=0; i<numRand; i++) {
						Particle p;
						randomParticle(ship.pos, p);
						particles.push_back(p);
					}
				}
			}

			//check against all other bullets
			for (int j=bullets.size()-1; j>=0; j--) {
				Bullet& b=bullets.at(j);

				if (a.containsPt(b.pos)) {
					//remove bullet
					bullets.erase(bullets.begin()+j);

					//if we can, split the asteroid
					int numRand;
					float2 pos;
					Asteroid newA, newB;
					if (a.split(newA, newB)) {
						asteroids.push_back(newA);
						asteroids.push_back(newB);
						//emit some particles for fx
						numRand=a.rad*Maths::random(2, 4);
						pos=newA.pos;

						//increment score
						score+=8;
					}
					else {//when we "fully" break an asteroid
						//emit more particles
						numRand=a.rad*Maths::random(5, 7);
						pos=a.pos;

						//increment score more
						score+=24;
					}
					for (int k=0; k<numRand; k++) {
						Particle p;
						randomParticle(pos, p);
						particles.push_back(p);
					}

					asteroids.erase(asteroids.begin()+i);
				}
			}
		}
		delete[] shipOutline;

		if (!lost) {
			//limit number of particles spawned
			if (particleTimer>0.003f) {
				particleTimer=0;
				if (boostKey) particles.push_back(ship.emitParticle());
			}
			particleTimer+=dt;

			//limit number of bullets shot
			if (bulletTimer>0.4f) {
				bulletTimer=0;
				if (getKey(VK_SPACE)) bullets.push_back(ship.getBullet());
			}
			bulletTimer+=dt;
		}

		//if "alive"
		if (!won&&!lost) {
			//award points for living longer
			if (scoreTimer>2) {
				scoreTimer=0;
				//based on how "hard" to live
				score+=asteroids.size();
			}
			scoreTimer+=dt;
		}

		//"leveling"
		if (asteroids.size()==0) {
			//win case
			if (stage==4) won=true;
			else {
				//blinking
				if (warningTimer>0.4f) {
					//reset
					warningTimer=0;
					if (warningStage<8) warningStage++;
					else {//add asteroids, next stage
						warningStage=0;

						//add asteroids based on stage
						for (int i=0; i<stage*2+1; i++) asteroids.push_back(randomAsteroid());
						stage++;
					}
				}
				warningTimer+=dt;
			}
		}

		//game over
		if (won||lost) {
			if (endTimer>0.7f) {
				endTimer=0;
				endStage++;
			}
		}
		endTimer+=dt;
	}

	void draw(Raster& rst) override {
		//background
		rst.setChar(' ');
		rst.fillRect(0, 0, width, height);

		//if debugging
		if (debugMode) {

			//line to all asteroids
			rst.setChar('.');
			rst.setColor(Raster::RED);
			for (Asteroid& a:asteroids) rst.drawLine(a.pos, ship.pos);

			//line to all bullets
			rst.setColor(Raster::GREEN);
			for (Bullet& b:bullets) rst.drawLine(b.pos, ship.pos);

			//show asteroids vel
			rst.setColor(Raster::BLUE);
			for (Asteroid& a:asteroids) rst.drawLine(a.pos, a.pos+a.vel);

			//show bullets vel
			for (Bullet& b:bullets) rst.drawLine(b.pos, b.pos+b.vel);

			//show ship vel
			rst.drawLine(ship.pos, ship.pos+ship.vel);

			//show asteroid bounds
			rst.setColor(Raster::GREY);
			for (Asteroid& a:asteroids) a.getAABB().render(rst);

			//show ship bounds
			ship.getAABB().render(rst);
		}

		//show particles
		rst.setColor(Raster::WHITE);
		for (Particle& p:particles) p.render(rst);

		//show all bullets
		rst.setChar('L');
		for (Bullet& b:bullets) {
			b.render(rst);
		}

		//show asteroids
		rst.setChar('a');
		for (Asteroid& a:asteroids) a.render(rst);

		if (!lost) {
			//show ship
			rst.setChar('S');
			ship.render(rst);
		}

		//show warning sign
		if (warningStage%2==1) {
			rst.setChar(' ');
			rst.fillRect(width/2-11, height/2-2, 22, 5);
			rst.setChar('#');
			rst.setColor(Raster::CYAN);
			rst.drawRect(width/2-11, height/2-2, 22, 5);
			rst.drawString(width/2-9, height/2, "ASTEROIDS IMMINENT");
		}

		//show endsign
		if (endStage%2==1) {
			rst.setChar(' ');
			rst.fillRect(width/2-11, height/2-2, 22, 5);
			if (won) {
				rst.setColor(Raster::GREEN);
				rst.drawString(width/2-4, height/2-1, "You Win!");
				std::string str="Score: "+std::to_string(score);
				rst.drawString(width/2-str.length()/2, height/2+1, str);
			}
			if (lost) {
				rst.setColor(Raster::RED);
				rst.drawString(width/2-5, height/2-1, "GAME OVER!");
				rst.drawString(width/2-10, height/2+1, "You hit an asteroid.");
			}
			rst.setChar('#');
			rst.drawRect(width/2-11, height/2-2, 22, 5);
		}

		//update title
		setTitle("Asteroids @ "+std::to_string((int)framesPerSecond)+"fps");

		//show stats
		rst.setColor(Raster::WHITE);
		rst.drawString(0, 0, "Score: "+std::to_string(score));
		rst.drawString(0, 1, "Stage: "+std::to_string(stage));

		//only when debugging
		if (debugMode) {
			//display asteroid stats
			rst.setColor(Raster::DARK_YELLOW);
			rst.drawString(width-13, 0, "Asteroid Data");
			for (int i=0; i<asteroids.size(); i++) {
				Asteroid& a=asteroids.at(i);
				//show at top right
				std::string posStr="[x:"+std::to_string(a.pos.x)+", y: "+std::to_string(a.pos.y)+"]";
				std::string str=std::to_string(i)+": [p: "+posStr+", n: "+std::to_string(a.numPts)+", r: "+std::to_string((int)a.rad)+"]";
				rst.drawString(width-str.length(), i+1, str);
			}
		}
	}
};

int main() {
	srand(time(NULL));

	//init custom graphics engine
	Demo d=Demo();
	d.startFullscreen(8);

	return 0;
}