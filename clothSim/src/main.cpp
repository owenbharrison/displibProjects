#include <vector>

#include "Engine.h"

#include "geom/AABB2D.h"
#include "maths/Maths.h"
using namespace displib;

struct ptc {
	float2 pos, vel, acc;
	bool locked=false;

	ptc() {}

	ptc(float2 pos_) {
		pos=pos_;
		vel=float2();
		acc=float2();
	}

	void update(float dt) {
		if (locked) return;
		vel+=acc*dt;
		pos+=vel*dt;
		acc*=0;
	}

	float getStress() {
		return length(vel)/25;
	}

	void applyForce(float2 f) { acc+=f; }
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
	float2 grav;
	int w, h;
	int ptcNum=0;
	ptc* ptcs;
	int sprNum=0;
	spr* sprs;
	//stress colors
	short* stressGrad=new short[8]{
		Raster::DARK_BLUE,
		Raster::BLUE,
		Raster::DARK_CYAN,
		Raster::CYAN,
		Raster::GREEN,
		Raster::DARK_YELLOW,
		Raster::RED,
		Raster::DARK_RED
	};

	ptc* heldPtc=nullptr;
	bool hold=false, wasHold=false;

	//call it stress, while it is really just the speed of the ptc
	short stressToCol(float s) {
		int csi=Maths::clamp(s*8, 0, 7);
		return stressGrad[csi];
	}

	//index func
	int ix(int i, int j) { return i+j*w; }

	void setup() override {
		grav=float2(0, 25);

		w=10;
		h=12;
		ptcNum=w*h;
		ptcs=new ptc[ptcNum];
		sprNum=2*w*h-w-h;
		sprs=new spr[sprNum];
		initCloth();
	}

	void initCloth() {
		//make softbody
		float stiff=437.243f;
		float damp=4.97f;
		for (int i=0; i<w; i++) {
			for (int j=0; j<h; j++) {
				//map to screen coords, with a little bit of random
				float x=Maths::map(i, 0, w-1, width/8, width*7/8)+Maths::random(-1, 1)/3;
				float y=Maths::map(j, 0, h-1, 0, height*3/5)+Maths::random(-1, 1)/3;
				ptc p(float2(x, y));
				//if at top corners, lock in place
				p.locked=(j==0&&(i==0||i==w-1));
				ptcs[ix(i, j)]=p;
			}
		}

		//connect axis aligned sprs
		int k=0;
		for (int i=0; i<w; i++) {
			for (int j=0; j<h; j++) {
				//connect right if can
				if (i<w-1) sprs[k++]=spr(ptcs[ix(i, j)], ptcs[ix(i+1, j)], stiff, damp);
				//connect down if can
				if (j<h-1) sprs[k++]=spr(ptcs[ix(i, j)], ptcs[ix(i, j+1)], stiff, damp);
			}
		}
	}

	void update(float dt) override {
		//option for reseting
		if (getKey('R')) initCloth();

		//add grav to particles
		for (int i=0; i<ptcNum; i++) ptcs[i].applyForce(grav);

		//to set and reset held ptc
		hold=getKey(' ');
		if (hold&&!wasHold) {
			float2 mp(mouseX, mouseY);
			for (int i=0; i<ptcNum; i++) {
				ptc& p=ptcs[i];
				if (length(p.pos-mp)<5) heldPtc=&p;
			}
		}
		if (!hold&&wasHold) heldPtc=nullptr;
		wasHold=hold;

		//to move held ptc
		if (heldPtc!=nullptr) {
			(*heldPtc).pos=float2(mouseX, mouseY);
		}

		//update springs
		for (int i=0; i<sprNum; i++) sprs[i].update();

		//euler explicit for particles
		for (int i=0; i<ptcNum; i++) ptcs[i].update(dt);

		grav.x=sinf(totalDeltaTime)*3;
	}

	void draw(Raster& rst) override {
		//background
		rst.setChar(' ');
		rst.fillRect(0, 0, width, height);

		//draw squares
		rst.setChar('*');
		for (int i=0; i<w-1; i++) {
			for (int j=0; j<h-1; j++) {
				//get 4 corners
				ptc& a=ptcs[ix(i, j)];
				ptc& b=ptcs[ix(i+1, j)];
				ptc& c=ptcs[ix(i, j+1)];
				ptc& d=ptcs[ix(i+1, j+1)];
				//color is avg between pts of square
				float avgStress=(a.getStress()+b.getStress()+c.getStress()+d.getStress())/4;
				rst.setColor(stressToCol(avgStress));
				//draw square
				rst.fillTriangle(a.pos, b.pos, c.pos);
				rst.fillTriangle(b.pos, d.pos, c.pos);
			}
		}

		//draw springs
		rst.setChar('#');
		for (int i=0; i<sprNum; i++) {
			spr& s=sprs[i];
			ptc& a=s.getA();
			ptc& b=s.getB();
			//color is avg between pts of spr
			float avgStress=(a.getStress()+b.getStress())/2;
			rst.setColor(stressToCol(avgStress));
			rst.drawLine(a.pos, b.pos);
		}

		//draw particles
		rst.setChar(0x2588);
		for (int i=0; i<ptcNum; i++) {
			ptc& p=ptcs[i];
			rst.setColor(stressToCol(p.getStress()));
			rst.putPixel(p.pos);
		}

		//show fps
		setTitle("Cloth Sim @ "+std::to_string((int)framesPerSecond)+"fps");
	}
};

int main() {
	srand(time(NULL));

	//init custom graphics engine
	Demo d=Demo();
	d.startWindowed(6, 80, 100);

	return 0;
}