#include <vector>

#include "Engine.h"
#include "maths/Maths.h"
#include "geom/AABB2D.h"
using namespace displib;

#define K 2

#define BOUNDARY .5f

struct ptc {
	float2 pos, oldpos, vel, acc;
	float rad;

	ptc(float2 pos_, float rad_) {
		pos=pos_;
		rad=rad_;
	}

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
};

class Demo : public Engine {
	public:
	int res, cols, rows;
	float* grid;

	int ix(int i, int j) {
		return i+j*cols;
	}

	void setup() override {
		res=5;
		cols=width/res+1;
		rows=height/res+1;
		grid=new float[cols*rows];
		for (int i=0; i<cols*rows; i++) { grid[i]=1; }
	}

	void update(float dt) override {
		float2 mousePos(mouseX, mouseY);

		bool add=getKey('A');
		bool remove=getKey('R');
		float2 ij=mousePos/res+.5f;
		for (int i=-K; i<=K; i++) {
			for (int j=-K; j<=K; j++) {
				int ri=ij.x+i;
				int rj=ij.y+j;
				if (ri>1&&rj>1&&ri<cols-2&&rj<rows-2) {
					float& val=grid[ix(ri, rj)];
					float amt=20.f/(i*i+j*j);
					//if (add) val+=amt*dt;
					if (remove) val-=amt*dt;
					if (val<0) val=0;
					if (val>1) val=1;
				}
			}
		}
	}

	void draw(Raster& rst) override {
		rst.setChar(' ');
		rst.fillRect(0, 0, width, height);

		std::vector<std::pair<float2, float2>> lines;
		auto line=[&lines](float2 a, float2 b) {
			lines.push_back({a, b});
		};
		for (int i=0; i<cols-1; i++) {
			for (int j=0; j<rows-1; j++) {
				float x=i*res;
				float y=j*res;
				float v0=grid[ix(i, j)];
				float v1=grid[ix(i+1, j)];
				float v2=grid[ix(i+1, j+1)];
				float v3=grid[ix(i, j+1)];
				int state=
					(v0>BOUNDARY)*8+
					(v1>BOUNDARY)*4+
					(v2>BOUNDARY)*2+
					(v3>BOUNDARY);
				float at=Maths::invLerp(BOUNDARY, v0, v1);
				float bt=Maths::invLerp(BOUNDARY, v1, v2);
				float ct=Maths::invLerp(BOUNDARY, v2, v3);
				float dt=Maths::invLerp(BOUNDARY, v3, v0);
				float2 a(x+res*at, y);
				float2 b(x+res, y+res*bt);
				float2 c(x+res*(1-ct), y+res);
				float2 d(x, y+res*(1-dt));
				switch (state) {
					case 1: case 14:
						line(c, d); break;
					case 2: case 13:
						line(b, c); break;
					case 3: case 12:
						line(b, d); break;
					case 4: case 11:
						line(a, b); break;
					case 5:
						line(a, d); line(b, c); break;
					case 6: case 9:
						line(a, c); break;
					case 7: case 8:
						line(a, d); break;
					case 10:
						line(a, b); line(c, d); break;
				};
			}
		}

		rst.setChar('#');
		for (int i=0; i<width; i++) {
			for (int j=0; j<height; j++) {
				float2 pt(i+.1, j+.1);
				float2 ptExt(pt.x+1, pt.y);
				int num=0;
				for (const auto& c:lines) {
					float t, u;
					Maths::lineLineIntersection(pt, ptExt, c.first, c.second, &t, &u);
					if (t>0&&u>0&&u<1) num++;
				}
				if (num%2==0) rst.putPixel(i, j);
			}
		}

		rst.setChar(0x2588);
		for (const auto& c:lines) {
			rst.drawLine(c.first, c.second);
		}
	}
};

int main() {
	srand(time(NULL));

	//init custom graphics engine
	Demo d;
	d.startFullscreen(6);

	return 0;
}