#include <vector>

#include "Engine.h"

#include "physics/Softbody.h"
#include "maths/Maths.h"
using namespace displib;

struct Poly {
	V2D pos;
	float angle=0;
	int num=0;
	V2D* mod=nullptr;
	V2D* pts=nullptr;

	void makeRect(V2D pos_, float w, float h, float angle_) {
		pos=pos_;
		angle=angle_;

		num=4;
		mod=new V2D[4]{
			V2D(-w/2, -h/2),
			V2D(w/2, -h/2),
			V2D(w/2, h/2),
			V2D(-w/2, h/2)
		};
		pts=new V2D[4];
	}

	void updatePts() {
		for (int i=0; i<num; i++) {
			V2D m=mod[i];
			pts[i]=V2D(
				(m.x*cosf(angle))-(m.y*sinf(angle)),
				(m.x*sinf(angle))+(m.y*cosf(angle))
			)+pos;
		}
	}

	AABB2D getAABB() {
		float nx=INFINITY;
		float ny=INFINITY;
		float mx=-INFINITY;
		float my=-INFINITY;
		for (int i=0; i<num; i++) {
			V2D p=pts[i];
			nx=min(nx, p.x);
			ny=min(ny, p.y);
			mx=max(mx, p.x);
			my=max(my, p.y);
		}
		return AABB2D(nx, ny, mx, my);
	}

	bool getClosestPt(V2D pt, V2D& closePt, V2D& norm) {
		//aabb optimization
		if (getAABB().containsPt(pt)) {
			//find pt in line? ray ix test
			int numIx=0;
			for (int i=0; i<num; i++) {
				V2D a=pts[i];
				V2D b=pts[(i+1)%num];
				float* tu=Maths::lineLineIntersection(a, b, pt, pt+V2D(1, 0));
				if (tu[0]>=0&&tu[0]<=1&&tu[1]>=0) numIx++;
			}
			//odd num of ix
			if (numIx%2==1) {
				float record=INFINITY;
				float realDist=0;
				int ei;
				for (int i=0; i<num; i++) {//find the closest edge
					V2D a=pts[i];
					V2D b=pts[(i+1)%num];
					//get dist to line
					V2D d=b-a;
					float top=d.x*(a.y-pt.y)-d.y*(a.x-pt.x);
					float dist=top/d.mag();
					float absDist=abs(dist);
					if (absDist<record) {//sorting
						record=absDist;
						realDist=dist;
						ei=i;
					}
				}
				//now we get point on closest line
				V2D sub=pts[(ei+1)%num]-pts[ei];
				V2D tang=V2D::normal(sub);
				norm=V2D(-tang.y, tang.x);
				closePt=pt+norm*realDist;
				return true;
			}
		}
		return false;
	}

	void render(Raster& rst) {
		for (int i=0; i<num; i++) {
			V2D a=pts[i];
			V2D b=pts[(i+1)%num];
			rst.drawLine(a.x, a.y, b.x, b.y);
		}
		rst.putPixel(pos.x, pos.y);
	}
};

class Demo : public Engine {
	public:
	V2D grav;
	AABB2D mouseBox;
	Softbody jello;
	float totalUpdateTime=0;
	std::vector<Poly> polys;

	V2D reflectVec(V2D d, V2D n) {
		return d-n*d.dot(n)*2;
	}

	void setup() override {
		grav=V2D(0, 21);

		float stiff=460.47f;
		float damp=6.23f;
		jello.formRectangle(7, 8, AABB2D(width*5/7, 0, width*6/7, height/4), stiff, damp);

		polys.push_back({});
		polys.at(0).makeRect(V2D(width*3/4, height*3/5), width/3, height/11, -Maths::PI/9);
		polys.at(0).updatePts();

		polys.push_back({});
		polys.at(1).makeRect(V2D(width/2, height*7/8), width/2, height/11, 0);
		polys.at(1).updatePts();

		polys.push_back({});
		polys.at(2).makeRect(V2D(width/4, height/2), width/24, height/2, 0);
		polys.at(2).updatePts();

		updateMouseBox();
	}

	void updateMouseBox() {
		mouseBox=AABB2D(mouseX-10, mouseY-8, mouseX+8, mouseY+10);
	}

	void update(float dt) override {
		if (getKey(VK_SPACE)) updateMouseBox();

		//add grav to all particles
		jello.applyForce(grav);

		//use euler explicit on all particles

		for (int i=0; i<jello.particleNum; i++) {
			Particle& ptc=jello.particles[i];

			for (Poly& ply:polys) {
				V2D pos, norm;
				if (ply.getClosestPt(ptc.pos, pos, norm)) {
					ptc.pos=pos;
					ptc.vel=reflectVec(ptc.vel, norm);
				}
			}
		}
		jello.update(dt);

		//make sure we are in the screen, out of the mouseBox
		jello.constrainOut(mouseBox);
	}

	void draw(Raster& rst) override {
		//background
		rst.setChar(' ');
		rst.fillRect(0, 0, width, height);

		rst.setChar('#');
		mouseBox.render(rst);
		for (Poly& p:polys) p.render(rst);

		//draw springs
		rst.setChar('j');
		jello.renderSprings(rst);

		//draw particles
		rst.setChar('*');
		jello.renderParticles(rst);

		//show fps
		setTitle("2D SoftBody sim @ "+std::to_string((int)framesPerSecond)+"fps");
	}
};

int main() {
	srand(time(NULL));

	//init custom graphics engine
	Demo d=Demo();
	d.startWindowed(3, 360, 240);

	return 0;
}
