#include <vector>
#include <strstream> 
#include <fstream>
#include <regex>
#include <algorithm>

#include "Engine.h"
#include "maths/Maths.h"
#include "maths/vector/float3.h"
using namespace displib;

//i really havent a clue where i got this
//i believe it is x and z matrix rotations, without z
float2 projfloat3(float3 v, float yaw, float pitch, float zoom) {
	return float2(
		sinf(yaw)*v.x-cosf(yaw)*v.z,
		(cosf(yaw)*v.x+sinf(yaw)*v.z)*cosf(pitch)+sinf(pitch)*v.y
	)*zoom;
}

int modulo(int x, int n) {
	return (x%n+n)%n;
}

struct tri {
	float3 a, b, c;
	bool lit=false;

	tri() {}

	tri(float3 a_, float3 b_, float3 c_) {
		a=a_;
		b=b_;
		c=c_;
	}

	float3 getNorm() {
		return normalize(cross(b-a, c-a));
	}

	float3 getAvgPos() {
		return (a+b+c)/3;
	}
};

struct cell {
	tri a, b;
	bool lit=false;

	cell() {}

	cell(tri a_, tri b_) {
		a=a_;
		b=b_;
	}

	void updateLit(bool lit_) {
		lit=lit_;
		a.lit=lit;
		b.lit=lit;
	}
};

class Demo : public Engine {
	public:
	float2 ctr;

	int wid, hei;
	float torusRad, tubeRad;
	int cellNum;
	cell* cells;
	float timer=2;

	float3 camPos;

	float camYaw=-0.983478f, camPitch=-1.922638f;
	float camZoom;

	int ix(int i, int j) {
		return i+j*wid;
	}

	void setup() override {
		ctr=float2(width/2, height/2);

		camZoom=height/2.2f;

		wid=36;
		hei=24;
		torusRad=0.75f;
		tubeRad=0.45f;
		float3* vtxs=new float3[wid*hei];

		//construct torus.
		for (int p=0; p<wid; p++) {
			float phi=Maths::map(p, 0, wid, 0, Maths::PI*2);

			for (int t=0; t<hei; t++) {
				float theta=Maths::map(t, 0, hei, 0, Maths::PI*2);

				//parametric torus
				float x=(torusRad+tubeRad*cosf(theta))*cosf(phi);
				float y=tubeRad*sinf(theta);
				float z=(torusRad+tubeRad*cosf(theta))*sinf(phi);

				vtxs[ix(p, t)]=float3(x, y, z);
			}
		}

		cellNum=wid*hei;
		cells=new cell[cellNum];
		for (int p=0; p<wid; p++) {
			int pn=(p+1)%wid;
			for (int t=0; t<hei; t++) {
				int tn=(t+1)%hei;
				float3 v00=vtxs[ix(p, t)];
				float3 v01=vtxs[ix(p, tn)];
				float3 v10=vtxs[ix(pn, t)];
				float3 v11=vtxs[ix(pn, tn)];
				cells[ix(p, t)]=cell(
					tri(v00, v01, v11),
					tri(v00, v11, v10)
				);
			}
		}
		delete[] vtxs;

		initCells();
	}

	void initCells() {
		for (int i=0; i<wid; i++) {
			for (int j=0; j<hei; j++) {
				cells[ix(i, j)].updateLit(Maths::random()>0.5f);
			}
		}
	}

	void update(float dt) override {
		//update "campos"
		camPos=float3(
			cosf(camYaw)*sinf(camPitch),
			-cosf(camPitch),
			sinf(camYaw)*sinf(camPitch)
		)*120;

		//slow
		float amt=Maths::PI/1.5f;
		if (getKey(VK_SHIFT)) amt/=5;
		//look dir
		if (getKey('W')) camPitch+=amt*dt;
		if (getKey('S')) camPitch-=amt*dt;
		if (getKey('A')) camYaw+=amt*dt;
		if (getKey('D')) camYaw-=amt*dt;
		camPitch=Maths::clamp(camPitch, -Maths::PI, 0);
		//zoom
		if (getKey(VK_UP)) camZoom+=30*dt;
		if (getKey(VK_DOWN)) camZoom-=30*dt;
		if (camZoom<1) camZoom=1;

		//for reset
		if (getKey('R')) initCells();

		//every so often
		if (timer>0.1f) {
			timer=0;

			//update cells
			bool* newValues=new bool[wid*hei];
			for (int i=0; i<wid; i++) {
				for (int j=0; j<hei; j++) {
					int numAlive=0;
					for (int i_=-1; i_<=1; i_++) {
						for (int j_=-1; j_<=1; j_++) {
							//dont check self
							if (!(i_==0&&j_==0)) {
								//wrap i
								int ri=modulo(i+i_, wid);
								//wrap j
								int rj=modulo(j+j_, hei);
								numAlive+=cells[ix(ri, rj)].lit;
							}
						}
					}

					//choose next timestep based on neighbors
					bool currAlive=cells[ix(i, j)].lit;
					bool nextAlive=false;
					//Any live cell with...
					if (currAlive) {
						//...fewer than two live neighbours dies, as if by underpopulation.
						if (numAlive<2) nextAlive=false;
						//...two or three live neighbours lives on to the next generation.
						else if (numAlive==2||numAlive==3) nextAlive=true;
						//...more than three live neighbours dies, as if by overpopulation.
						else if (numAlive>3) nextAlive=false;
					}
					//...exactly three live neighbours becomes a live cell, as if by reproduction.
					else if (numAlive==3) nextAlive=true;

					newValues[ix(i, j)]=nextAlive;
				}
			}

			//copy next to current
			for (int i=0; i<cellNum; i++) {
				cells[i].updateLit(newValues[i]);
			}

			//clear temp storage
			delete[] newValues;
		}
		//update timer
		timer+=dt;

		//update title
		setTitle("3D Automata @ "+std::to_string((int)framesPerSecond)+"fps");
	}

	void draw(Raster& rst) override {
		//background
		rst.setChar(0x2588);
		rst.setColor(Raster::DARK_GREY);
		rst.fillRect(0, 0, width, height);

		//"optimization" show only front facing tris
		std::vector<tri> trisToDraw;
		for (int i=0; i<cellNum; i++) {
			cell& c=cells[i];
			//culling
			if (dot(c.a.getNorm(), c.a.a-camPos)<0) trisToDraw.push_back(c.a);
			if (dot(c.b.getNorm(), c.b.a-camPos)<0) trisToDraw.push_back(c.b);
		}

		//"painters"-ish algo to sort by what is closer to the "camera"
		sort(trisToDraw.begin(), trisToDraw.end(), [&](tri& a, tri& b) {
			return length(a.getAvgPos()-camPos)>length(b.getAvgPos()-camPos);
		});

		//"project" tris
		bool drawOutline=!getKey('O');
		rst.setColor(Raster::CYAN);
		for (tri& t:trisToDraw) {
			//get projected coords
			float2 a=projfloat3(t.a, camYaw, camPitch, camZoom)+ctr;
			float2 b=projfloat3(t.b, camYaw, camPitch, camZoom)+ctr;
			float2 c=projfloat3(t.c, camYaw, camPitch, camZoom)+ctr;

			//show based on "lit"
			rst.setChar(t.lit?0x2588:' ');
			rst.fillTriangle(a, b, c);

			//draw outline
			if (drawOutline) {
				rst.setChar('.');
				rst.drawTriangle(a, b, c);
			}
		}
	}
};

int main() {
	srand(time(NULL));

	Demo d;
	d.startWindowed(4, 240, 200);

	return 0;
}