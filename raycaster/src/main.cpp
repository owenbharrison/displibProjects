#include <vector>

#include "Engine.h"
#include "maths/Maths.h"
using namespace displib;

struct Ray {
	V2D origin, dir;
};

struct Line {
	V2D a, b;
	short col;

	float intersectRay(Ray r) {
		float* tu=Maths::lineLineIntersection(a, b, r.origin, r.origin+r.dir);
		float t;
		if (tu[0]>0&&tu[0]<1&&tu[1]>0) t=tu[1];//ray line intersect
		else t=-1;//no intersect
		delete[] tu;
		return t;
	}
};

class Demo : public Engine {
	public:
	std::vector<Line> lines;
	float MAX_DIST;
	V2D camPos;
	float FOV=Maths::PI*2/3;
	float camAngle=0;

	void setup() override {
		//screen boundaries
		lines.push_back({V2D(-1, -1), V2D(width, -1)});
		lines.push_back({V2D(width, -1), V2D(width, height)});
		lines.push_back({V2D(width, height), V2D(-1, height)});
		lines.push_back({V2D(-1, height), V2D(-1, -1)});

		//randomly spawn lines on screen
		int num=Maths::random(5, 9);
		float bfr=width/11;
		for (int i=0; i<num; i++) {
			float x0=Maths::random(bfr, width-bfr);
			float y0=Maths::random(bfr, height-bfr);
			float x1=Maths::random(bfr, width-bfr);
			float y1=Maths::random(bfr, height-bfr);
			//random color
			short col=Maths::random(16);
			lines.push_back({V2D(x0, y0), V2D(x1, y1), col});
		}

		MAX_DIST=sqrtf(width*width+height*height);
		camPos=V2D(width/2, height/2);
	}

	void shutdown() override {
		lines.clear();
	}

	void update(float dt) override {
		//change pos with keys:
		//move forward and back
		float spd=width/6.5f;
		V2D fb=V2D::fromAngle(camAngle)*spd;
		if (getKey('W')) camPos+=fb*dt;
		else if (getKey('S')) camPos-=fb*dt;

		//turn left and right
		if (getKey('A')) camAngle-=2*dt;
		else if (getKey('D')) camAngle+=2*dt;
	}

	void draw(Raster& rst) override {
		//background
		rst.setChar(' ');
		rst.fillRect(0, 0, width, height);

		//solve and draw rays
		int num=36;
		rst.setChar(0x2588);
		rst.setColor(Raster::WHITE);
		for (int i=0; i<num; i++) {
			float angle=Maths::map(i, 0, num, -FOV/2, FOV/2)+camAngle;
			V2D dir=V2D::fromAngle(angle);
			Ray ray={camPos, dir};

			//sort by closest line
			float record=INFINITY;
			Line* chosenLine=nullptr;
			for (Line& line:lines) {
				float distToLine=line.intersectRay(ray);
				//check validity and if closest
				if(distToLine>0&&distToLine<record) {
					record=distToLine;
					chosenLine=&line;
				}
			}

			//if valid
			float dist=MAX_DIST;
			if (chosenLine!=nullptr) {
				dist=record;
			}
			V2D endPt=ray.origin+ray.dir*dist;
			rst.drawLine(ray.origin.x, ray.origin.y, endPt.x, endPt.y);
		}

		//draw "player" pos
		rst.setChar('@');
		rst.setColor(Raster::CYAN);
		rst.putPixel(camPos.x, camPos.y);

		//draw "lines"
		rst.setChar('#');
		for (Line& l:lines) {
			rst.setColor(l.col);
			rst.drawLine(l.a.x, l.a.y, l.b.x, l.b.y);
		}

		//show fps
		rst.setChar(' ');
		rst.fillRect(0, 0, 10, 2);
		rst.setColor(Raster::WHITE);
		rst.drawString(0, 0, "FPS: "+std::to_string((int)framesPerSecond));
	}
};

int main() {
	srand(time(NULL));

	//init custom graphics engine
	Demo d=Demo();
	d.start(8, 8, true);

	return 0;
}
