#include "Engine.h"
#include "escapi.h"
#include "maths/Maths.h"
using namespace displib;

class Demo : public Engine {
	public:
	const char* asciiArr=" .,~=#&@";
	SimpleCapParams capture;
	short colors[8]={
		Raster::BLACK,
		Raster::BLUE,
		Raster::DARK_GREEN,
		Raster::CYAN,
		Raster::DARK_RED,
		Raster::DARK_MAGENTA,
		Raster::DARK_YELLOW,
		Raster::WHITE
	};

	void setup() override {
		//init escapi
		int devices=setupESCAPI();

		if (devices==0) {
			printf("NO DEVICE FOUND");
			shutdown();
		}

		capture.mWidth=width;
		capture.mHeight=height;
		capture.mTargetBuf=new int[width*height];

		//init capture
		if (initCapture(0, &capture)==0) {
			printf("DEVICE IN USE");
			shutdown();
		}
	}

	void shutdown() override {
		deinitCapture(0);
	}

	void draw(Raster& rst) override {
		doCapture(0);
		//wait until done
		while (isCaptureDone(0)==0) {}

		rst.setChar('#');
		for (int i=0; i<width; i++) {
			for (int j=0; j<height; j++) {
				union BGRint {
					int bgr;
					unsigned char c[4];
				};

				BGRint col;
				col.bgr=capture.mTargetBuf[i+j*width];
				//int brightness=(col.c[2]+col.c[1]+col.c[1])/3;
				//float pct=brightness/255.0f;
				//int asi=Maths::clamp(pct*8, 0, 7);
				//rst.setChar(asciiArr[asi]);

				//make rgb into 8 bit color (binary "interp")
				int bin=(col.c[2]>127)*4+(col.c[1]>127)*2+(col.c[0]>127);
				rst.setColor(colors[bin]);
				rst.putPixel(i, j);
			}
		}

		//show fps
		rst.setChar(' ');
		rst.fillRect(0, 0, 10, 2);
		rst.setColor(Raster::WHITE);
		rst.drawString(0, 0, "FPS: "+std::to_string((int)framesPerSecond));
	}
};

int main() {
	//init custom graphics engine
	Demo d=Demo();
	d.start(12, 12, true);

	return 0;
}