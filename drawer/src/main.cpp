#include <time.h>

#include "Engine.h"
using namespace displib;

class Demo : public Engine {
	public:
	short currColor;
	float timer=0;
	int pmx, pmy, rmx, rmy;
	short currSize=1;

	void setup() override {
		currColor=Raster::WHITE;
	}

	void update(float dt) override {
		//cursor size
		for (int i=0; i<10; i++) {
			if (getKey('0'+i)) {
				currSize=i;
			}
		}
		//color
		if (getKey('R')) currColor=Raster::DARK_RED;
		if (getKey('Y')) currColor=Raster::DARK_YELLOW;
		if (getKey('L')) currColor=Raster::GREEN;
		if (getKey('B')) currColor=Raster::BLUE;
		if (getKey('M')) currColor=Raster::MAGENTA;
		if (getKey('W')) currColor=Raster::WHITE;
		if (getKey('G')) currColor=Raster::DARK_GREY;
		if (getKey('X')) currColor=Raster::BLACK;

		//delayed mouse positioning
		if (timer>1/60) {
			//reset
			timer=0;

			pmx=rmx;
			pmy=rmy;
			rmx=mouseX;
			rmy=mouseY;
		}

		//always update
		timer+=dt;
	}

	void draw(Raster& rst) override {
		//on first round
		if (updateCount==1) {
			//background
			rst.setChar(' ');
			rst.fillRect(0, 0, width, height);
		}

		//hold to draw
		if (getKey(VK_SPACE)) {
			//solid block char
			rst.setChar(0x2588);
			rst.setColor(currColor);
			//draw big stroke
			for (int i=-currSize/2; i<=currSize/2; i++) {
				for (int j=-currSize/2; j<=currSize/2; j++) {
					rst.drawLine(pmx+i, pmy+j, rmx+i, rmy+j);
				}
			}
		}

		//show cursor size
		rst.setChar('0'+currSize);
		rst.setColor(Raster::WHITE);
		rst.putPixel(0, 0);
	}
};

int main() {
	srand(time(NULL));

	//init custom graphics engine
	Demo d=Demo();
	d.startFullscreen(12);

	return 0;
}