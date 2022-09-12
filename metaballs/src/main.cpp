#include <time.h>

#include "Engine.h"
#include "maths/Maths.h"
#include "geom/AABB2D.h"
using namespace displib;

struct Metaball {
	float2 pos, vel;
	float rad=0;

	void update(float dt) {
		//euler explicit integ.
		pos+=vel*dt;
	}

	void checkAABB(AABB2D a) {
		//bounds detect with bouncing
		if (pos.x<a.min.x) { pos.x=a.min.x; vel.x*=-1; }
		if (pos.y<a.min.y) { pos.y=a.min.y; vel.y*=-1; }
		if (pos.x>a.max.x) { pos.x=a.max.x; vel.x*=-1; }
		if (pos.y>a.max.y) { pos.y=a.max.y; vel.y*=-1; }
	}
};

class Demo : public Engine {
	public:
	AABB2D bounds;
	int num=6;
	Metaball* metaballs;

	short colorArr[10]={
		Raster::DARK_RED,
		Raster::DARK_YELLOW,
		Raster::GREEN,
		Raster::DARK_GREEN,
		Raster::CYAN,
		Raster::DARK_CYAN,
		Raster::BLUE,
		Raster::DARK_BLUE,
		Raster::MAGENTA,
		Raster::DARK_MAGENTA
	};
	int colorLen=10;

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
			metaballs[i]={float2(x, y), float2(cosf(angle), sinf(angle))*spd, rad};
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
		rst.setChar('#');
		bool* grid=new bool[width*height];
		for (int x=0; x<width; x++) {
			for (int y=0; y<height; y++) {
				//sum all "radius strengths"
				float sum=0;
				for (int i=0; i<num; i++) {
					float2 sb=float2(x, y)-metaballs[i].pos;
					float r=metaballs[i].rad;
					sum+=r*r/dot(sb, sb);
				}
				//make 0-1 float into ascii ramp value
				float pct=Maths::clamp(sum/8, 0, 1);
				int asi=Maths::clamp(pct*colorLen, 0, colorLen-1);
				rst.setColor(colorArr[asi]);
				rst.putPixel(x, y);

				//put val into grid for edge detect.
				float amt=Maths::map(sinf(totalDeltaTime), -1, 1, 0.3f, 5.5f);
				grid[ix(x, y)]=sum>amt;
			}
		}

		//edge detection
		rst.setChar(' ');
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

		//show fps
		rst.fillRect(0, 0, 10, 2);
		rst.setColor(Raster::WHITE);
		rst.drawString(0, 0, "FPS: "+std::to_string((int)framesPerSecond));
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
	d.startFullscreen(10);

	return 0;
}