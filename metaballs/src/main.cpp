#include <time.h>

#include "Engine.h"
#include "maths/Maths.h"
#include "geom/AABB2D.h"
using namespace displib;

struct Metaball {
	V2D pos, vel;
	float rad=0.0f;

	void update(float dt) {
		//euler explicit integ.
		pos+=vel*dt;
	}

	void checkAABB(AABB2D a) {
		//bounds detect with bouncing
		if (pos.x<a.min.x) {
			pos.x=a.min.x;
			vel.x*=-1.0f;
		}
		if (pos.y<a.min.y) {
			pos.y=a.min.y;
			vel.y*=-1.0f;
		}
		if (pos.x>a.max.x) {
			pos.x=a.max.x;
			vel.x*=-1.0f;
		}
		if (pos.y>a.max.y) {
			pos.y=a.max.y;
			vel.y*=-1.0f;
		}
	}
};

class Demo : public Engine {
	public:
	AABB2D bounds;
	int num=6;
	Metaball* metaballs;
	const char* asciiArr=" .=+#@";
	const int asciiLen=strlen(asciiArr);

	void setup() override {
		bounds=AABB2D(0, 0, width, height);

		metaballs=new Metaball[num];
		//put randomly on screen
		for (int i=0; i<num; i++) {
			float x=Maths::random(0, width);
			float y=Maths::random(0, height);
			float angle=Maths::random(-Maths::PI, Maths::PI);
			float spd=Maths::random(6, 12);
			float rad=Maths::random(12, 22);
			metaballs[i]={V2D(x, y), V2D::fromAngle(angle)*spd, rad};
		}
	}

	void update(float dt) override {
		//update all
		for (int i=0; i<num; i++) {
			metaballs[i].update(dt);
			metaballs[i].checkAABB(bounds);
		}
	}

	void draw(Raster& rst) override {
		//background
		rst.setChar(' ');
		rst.fillRect(0, 0, width, height);

		//metaball method
		bool* grid=new bool[width*height];
		for (int x=0; x<width; x++) {
			for (int y=0; y<height; y++) {
				//sum all "radius strengths"
				float sum=0.0f;
				for (int i=0; i<num; i++) {
					V2D sb=V2D(x, y)-metaballs[i].pos;
					float r=metaballs[i].rad;
					sum+=r*r/sb.dot(sb);
				}
				//make 0-1 float into ascii ramp value
				float pct=Maths::clamp(sum/8, 0, 1);
				int asi=Maths::clamp(pct*asciiLen, 0, asciiLen-1);
				rst.setChar(asciiArr[asi]);
				rst.putPixel(x, y);

				//put val into grid for edge detect.
				float amt=Maths::map(sinf(totalDt), -1, 1, 0.5f, 6.5f);
				grid[ix(x, y)]=sum>amt;
			}
		}

		//edge detection
		rst.setChar('#');
		rst.setColor(Raster::GREEN);
		for (int x=0; x<width; x++) {
			for (int y=0; y<height; y++) {
				bool diff=false;
				bool curr=grid[ix(x, y)];
				if (x>1) diff|=(curr!=grid[ix(x-1, y)]);//left or
				if (y>1) diff|=(curr!=grid[ix(x, y-1)]);//up or
				if (x<width-2) diff|=(curr!=grid[ix(x+1, y)]);//down or
				if (y<height-2) diff|=(curr!=grid[ix(x, y+1)]);//right
				if (diff) rst.putPixel(x, y);
			}
		}
		delete[] grid;

		//show "ball" positions
		rst.setColor(Raster::CYAN);
		for (int i=0; i<num; i++) {
			Metaball& m=metaballs[i];
			rst.putPixel(m.pos.x, m.pos.y);
		}

		//show fps
		rst.setChar(' ');
		rst.fillRect(0, 0, 10, 2);
		rst.setColor(Raster::WHITE);
		rst.drawString(0, 0, "FPS: "+std::to_string((int)fps));
	}

	//2d array "hack" index method
	int ix(int i, int j) {
		return i+j*width;
	}
};

int main() {
	srand(time(NULL));

	//init custom graphics engine
	Demo d=Demo();
	d.start(12, 12, true);

	return 0;
}