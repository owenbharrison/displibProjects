#include <time.h>

#include "Engine.h"
#include "maths/Maths.h"
using namespace displib;

class Demo : public Engine {
	public:
	float2 ptA, ptB;
	int mainNum;
	float2* mainPoly;

	bool clipPolyByLine(float2* polyPts, int numPts, float2 lp0, float2 lp1, float2*& ptsAOut, int& sizeAOut, float2*& ptsBOut, int& sizeBOut) {
		//this has to be at least a triangle to be a polygon.
		if (numPts<3) return false;

		float2 ptMin, ptMax;
		int iMin=-1, iMax=-1;
		int numIx=0;
		//loop through the poly
		for (int i=0; i<numPts; i++) {
			//get the "edge" at the index
			float2 a=polyPts[i];
			float2 b=polyPts[(i+1)%numPts];
			//find line intersections
			float t, u;
			Maths::lineLineIntersection(a, b, lp0, lp1, &t, &u);
			float2 ixPt=a+(b-a)*t;
			//if the hit is inbetween both lines
			if (t>0&&t<1&&u>0&&u<1) {
				//set the pts and indexes
				if (iMin<0) { iMin=i; ptMin=ixPt; }
				else if (iMax<0) { iMax=i; ptMax=ixPt; }
				numIx++;
			}
		}

		//check to "make sure" poly is convex or that the line is "properly" slicing it
		if (numIx!=2) return false;

		//poly a
		int sizeA=iMax-iMin+2;
		sizeAOut=sizeA;
		ptsAOut=new float2[sizeA];
		int iA=0;
		ptsAOut[iA++]=ptMax;
		ptsAOut[iA++]=ptMin;
		//one side of the orig poly
		for (int n=iMin; n!=iMax; n=(n+1)%numPts) {//"circular" "index-stepping"
			ptsAOut[iA++]=polyPts[(n+1)%numPts];
		}

		//poly b
		int sizeB=numPts+4-sizeA;
		sizeBOut=sizeB;
		ptsBOut=new float2[sizeB];
		int iB=0;
		ptsBOut[iB++]=ptMin;
		ptsBOut[iB++]=ptMax;
		//the other side of the orig poly
		for (int n=iMax; n!=iMin; n=(n+1)%numPts) {//"circular" "index-stepping"
			ptsBOut[iB++]=polyPts[(n+1)%numPts];
		}

		//done!
		return true;
	}

	void drawPolyWithAllLines(Raster& rst, float2* pts, int numPts) {
		//connect each pt to every other point with a line
		for (int i=0; i<numPts; i++) {
			for (int j=i+1; j<numPts; j++) {
				float2 a=pts[i];
				float2 b=pts[j];
				rst.drawLine(a, b);
			}
		}
	}

	void setup() override {
		setTitle("Simple Polygon Clipping");

		//some randomly sized poly on the screen
		mainNum=Maths::clamp(rand()%7+3, 3, 10);
		mainPoly=new float2[mainNum];
		float rad=Maths::random(height/6, height/3);
		float2 pos(
			Maths::random(rad, width-rad),
			Maths::random(rad, height-rad)
		);
		for (int i=0; i<mainNum; i++) {
			float angle=Maths::map(i, 0, mainNum, 0, Maths::TAU);
			mainPoly[i]=float2(cosf(angle), sinf(angle))*rad+pos;
		}
	}

	void update(float dt) override {
		float2 mv(mouseX, mouseY);
		//user input
		if (getKey('1')) ptA=mv;
		if (getKey('2')) ptB=mv;
	}

	void draw(Raster& rst) override {
		//background
		rst.setChar(' ');
		rst.setColor(Raster::WHITE);
		rst.fillRect(0, 0, width, height);

		//draw polygon
		rst.setChar('p');
		rst.setColor(Raster::BLUE);
		for (int i=0; i<mainNum; i++) {
			float2 a=mainPoly[i];
			float2 b=mainPoly[(i+1)%mainNum];
			rst.drawLine(a, b);
		}

		//"clip" polygon
		float2* polyA, * polyB;
		int sizeA=0, sizeB=0;
		bool clipped=clipPolyByLine(mainPoly, mainNum, ptA, ptB, polyA, sizeA, polyB, sizeB);

		//only if the poly was "clippable"
		if (clipped) {
			//draw poly a
			rst.setChar('a');
			rst.setColor(Raster::RED);
			drawPolyWithAllLines(rst, polyA, sizeA);

			//draw poly b
			rst.setChar('b');
			rst.setColor(Raster::DARK_YELLOW);
			drawPolyWithAllLines(rst, polyB, sizeB);
			delete[] polyA;
			delete[] polyB;
		}

		//draw the clipline
		rst.setChar('c');
		rst.setColor(Raster::GREEN);
		rst.drawLine(ptA, ptB);

		//draw clipline pt 1
		rst.setChar('1');
		rst.setColor(Raster::WHITE);
		rst.fillRect(ptA.x-1, ptA.y-1, 3, 3);

		//draw clipline pt 2
		rst.setChar('2');
		rst.fillRect(ptB.x-1, ptB.y-1, 3, 3);
	}
};

int main() {
	srand(time(NULL));

	//init custom graphics engine
	Demo d=Demo();
	d.startWindowed(4, 200, 200);

	return 0;
}