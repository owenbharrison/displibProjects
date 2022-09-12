#include "Engine.h"
#include "maths/Maths.h"
#include "geom/AABB2D.h"
using namespace displib;

struct voronoiPt {
	float x, y, vx, vy;

	void update(float dt) {
		//euler explicit integ.
		x+=vx*dt;
		y+=vy*dt;
	}

	void checkAABB(AABB2D a) {
		//bounds detection and bounce
		if (x<a.min.x) { x=a.min.x; vx*=-1; }
		if (y<a.min.y) { y=a.min.y; vy*=-1; }
		if (x>a.max.x) { x=a.max.x; vx*=-1; }
		if (y>a.max.y) { y=a.max.y; vy*=-1; }
	}
};

class Demo : public Engine {
	public:
	int num=16;
	voronoiPt* vnPts;
	AABB2D bounds;
	bool manhattanDistance=false;
	bool renderFlip=false, wasRenderFlip=false;

	void setup() override {
		vnPts=new voronoiPt[num];
		//put randomly on screen
		for (int i=0; i<num; i++) {
			float x=Maths::random(0, width);
			float y=Maths::random(0, height);
			float angle=Maths::random(-Maths::PI, Maths::PI);
			float spd=Maths::random(5, 11);
			float2 vel=float2(cosf(angle), sinf(angle))*spd;
			vnPts[i]={x, y, vel.x, vel.y};
		}

		bounds=AABB2D(0, 0, width, height);
	}

	void update(float dt) override {
		//update all
		for (int i=0; i<num; i++) {
			vnPts[i].update(dt);
			vnPts[i].checkAABB(bounds);
		}

		renderFlip=getKey(VK_RETURN);
		if (renderFlip&&!wasRenderFlip) {
			manhattanDistance=!manhattanDistance;
		}
		wasRenderFlip=renderFlip;
	}

	void draw(Raster& rst) override {
		//background
		rst.setChar(' ');
		rst.fillRect(0, 0, width, height);

		//screen sized "2d" array
		int* grid=new int[width*height];
		//for every pixel
		for (int x=0; x<width; x++) {
			for (int y=0; y<height; y++) {
				//sorting
				float record=INFINITY;
				int closestIx=-1;
				//find closest pt
				for (int i=0; i<num; i++) {
					float dx=x-vnPts[i].x;
					float dy=y-vnPts[i].y;
					float ds=manhattanDistance?abs(dx)+abs(dy):sqrt(dx*dx+dy*dy);
					if (ds<record) {
						record=ds;
						closestIx=i;
					}
				}
				//store index of closest pt
				grid[x+y*width]=closestIx;
			}
		}

		//show "cells"
		for (int x=0; x<width; x++) {
			for (int y=0; y<height; y++) {
				//set char to a-z weird pattern
				rst.setChar(97+(x+y)%26);
				//color each cell accordingly
				rst.setColor(grid[x+y*width]);
				rst.putPixel(x, y);
			}
		}

		//edge detection
		rst.setChar(' ');
		for (int x=0; x<width; x++) {
			for (int y=0; y<height; y++) {
				bool diff=false;
				int curr=grid[x+y*width];
				//"highlight" any differences between pixels
				if (x>1) diff|=(curr!=grid[x-1+y*width]);//left or
				if (y>1) diff|=(curr!=grid[x+y*width-width]);//up or
				if (x<width-2) diff|=(curr!=grid[x+1+y*width]);//right or
				if (y<height-2) diff|=(curr!=grid[x+y*width+width]);//down
				if (diff) rst.putPixel(x, y);
			}
		}
		delete[] grid;
	}
};

int main() {
	srand(time(NULL));

	//init custom graphics engine
	Demo d;
	d.startFullscreen(8);

	return 0;
}