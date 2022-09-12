#include <vector>

#include "Engine.h"

#include "geom/AABB2D.h"
#include "maths/Maths.h"
using namespace displib;

struct ptc {
	float2 pos, vel, acc;

	ptc() {}

	ptc(float2 pos_) {
		pos=pos_;
		vel=float2();
		acc=float2();
	}

	void update(float dt) {
		vel+=acc*dt;
		pos+=vel*dt;
		acc*=0;
	}

	void applyForce(float2 f) { acc+=f; }

	void show(Raster& rst) { rst.putPixel(pos); }
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

struct poly {
	float2 pos;
	float angle=0;
	int num=0;
	float2* mod=nullptr;
	float2* pts=nullptr;

	poly(float2 pos_, float w, float h, float angle_) {
		pos=pos_;
		angle=angle_;

		num=4;
		mod=new float2[4]{
			float2(-w/2, -h/2),
			float2(w/2, -h/2),
			float2(w/2, h/2),
			float2(-w/2, h/2)
		};
		pts=new float2[4];

		updatePts();
	}

	void updatePts() {
		for (int i=0; i<num; i++) {
			float2 m=mod[i];
			pts[i]=float2(
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
			float2 p=pts[i];
			nx=min(nx, p.x);
			ny=min(ny, p.y);
			mx=max(mx, p.x);
			my=max(my, p.y);
		}
		return AABB2D(nx, ny, mx, my);
	}

	bool getClosestPt(float2 pt, float2& closePt, float2& norm) {
		//aabb optimization
		if (getAABB().containsPt(pt)) {
			//find pt in line? ray ix test
			int numIx=0;
			for (int i=0; i<num; i++) {
				float2 a=pts[i];
				float2 b=pts[(i+1)%num];
				float t, u;
				Maths::lineLineIntersection(a, b, pt, pt+float2(1, 0), &t, &u);
				if (t>=0&&t<=1&&u>=0) numIx++;
			}
			//odd num of ix
			if (numIx%2==1) {
				float record=INFINITY;
				float realDist=0;
				int ei;
				for (int i=0; i<num; i++) {//find the closest edge
					float2 a=pts[i];
					float2 b=pts[(i+1)%num];
					//get dist to line
					float2 d=b-a;
					float top=d.x*(a.y-pt.y)-d.y*(a.x-pt.x);
					float dist=top/length(d);
					float absDist=abs(dist);
					if (absDist<record) {//sorting
						record=absDist;
						realDist=dist;
						ei=i;
					}
				}
				//now we get point on closest line
				float2 sub=pts[(ei+1)%num]-pts[ei];
				float2 tang=normalize(sub);
				norm=float2(-tang.y, tang.x);
				closePt=pt+norm*realDist;
				return true;
			}
		}
		return false;
	}

	void show(Raster& rst) {
		for (int i=0; i<num; i++) {
			float2 a=pts[i];
			float2 b=pts[(i+1)%num];
			rst.drawLine(a, b);
		}
		rst.putPixel(pos);
	}
};

class Demo : public Engine {
	public:
	float2 grav;
	int ptcNum=0;
	ptc* ptcs={};
	int sprNum=0;
	spr* sprs={};
	float totalUpdateTime=0;
	std::vector<poly> polys;

	float2 reflectVec(float2 d, float2 n) {
		return d-n*dot(d, n)*2;
	}

	void setup() override {
		grav=float2(0, 32);

		//make softbody
		float stiff=560.47f;
		float damp=4.23f;
		int w=7;
		int h=5;
		auto ix=[&](int i, int j) { return i+j*w; };
		ptcNum=w*h;
		ptcs=new ptc[ptcNum];
		for (int i=0; i<w; i++) {
			for (int j=0; j<h; j++) {
				//evenly distribute
				float x=Maths::map(i, 0, w-1, width/5, width/3);
				float y=Maths::map(j, 0, h-1, height/8, height/4);
				ptcs[ix(i, j)]=ptc(float2(x, y));
			}
		}

		//connect axis aligned springs
		int k=0;
		//complex index maths worked out by hand
		sprNum=4*w*h-3*w-3*h+2;
		sprs=new spr[sprNum];
		for (int i=0; i<w; i++) {
			for (int j=0; j<h; j++) {
				//connect right if can
				if (i<w-1) sprs[k++]=spr(ptcs[ix(i, j)], ptcs[ix(i+1, j)], stiff, damp);
				//connect down if can
				if (j<h-1) sprs[k++]=spr(ptcs[ix(i, j)], ptcs[ix(i, j+1)], stiff, damp);
			}
		}
		//connect diags springs
		for (int i=0; i<w-1; i++) {
			for (int j=0; j<h-1; j++) {
				//connect to opposite corners
				sprs[k++]=spr(ptcs[ix(i, j)], ptcs[ix(i+1, j+1)], stiff, damp);
				sprs[k++]=spr(ptcs[ix(i+1, j)], ptcs[ix(i, j+1)], stiff, damp);
			}
		}

		//add polys to "scene"
		polys.clear();
		//left slant
		polys.push_back(poly(float2(width/3, height*2/5), width/3, height/12, Maths::PI/11));

		//right slant
		polys.push_back(poly(float2(width*3/4, height*5/9), width/3, height/11, -Maths::PI/6));

		//bottom
		polys.push_back(poly(float2(width*3/7, height*6/7), width/2, height/11, Maths::PI/13));
	}

	void update(float dt) override {
		//add grav to particles
		for (int i=0; i<ptcNum; i++) ptcs[i].applyForce(grav);

		//check each poly v particle, resolve collision
		for (int i=0; i<ptcNum; i++) {
			ptc& p=ptcs[i];

			for (poly& ply:polys) {
				float2 pos, norm;
				if (ply.getClosestPt(p.pos, pos, norm)) {
					p.pos=pos;
					p.vel=reflectVec(p.vel, norm);
				}
			}
		}

		//update springs
		for (int i=0; i<sprNum; i++) sprs[i].update();

		//euler explicit for particles
		for (int i=0; i<ptcNum; i++) ptcs[i].update(dt);

		bool allOut=true;
		for (int i=0; i<ptcNum; i++) {
			if (ptcs[i].pos.y<height) allOut=false;
		}
		if (allOut) setup();
	}

	void draw(Raster& rst) override {
		//background
		rst.setChar(' ');
		rst.fillRect(0, 0, width, height);

		//draw polys
		rst.setChar('#');
		for (poly& p:polys) p.show(rst);

		//draw springs
		rst.setChar('j');
		for (int i=0; i<sprNum; i++) sprs[i].show(rst);

		//draw particles
		rst.setChar('*');
		for (int i=0; i<ptcNum; i++) ptcs[i].show(rst);

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
