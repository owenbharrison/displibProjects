#include <time.h>
#include <algorithm>

#include "Engine.h"
#include "maths/Maths.h"
using namespace displib;

#include <noise/noise.h>
using namespace noise;

class Demo : public Engine {
	public:
	float zInc=0;
	module::Perlin perlinNoise;
	const char* asciiArr=" .,~=#&@";

	void setup() override {
		perlinNoise.SetSeed(time(NULL));
	}

	void update(float dt) override {
		zInc+=dt/4;
	}

	void draw(Raster& rst) override {
		//background
		rst.setChar(' '); 
		rst.fillRect(0, 0, width, height);

		float xInc=0;
		for (int x=0; x<width; x++) {
			xInc+=0.01f;
			float yInc=0;
			for (int y=0; y<height; y++) {
				yInc+=0.01f;
				float val=perlinNoise.GetValue(xInc, yInc+100, zInc-100);
				int asi=Maths::clamp(val*8, 0, 7);
				rst.setChar(asciiArr[asi]);
				rst.putPixel(x, y);
			}
		}
	}
};

int main() {
	//init custom graphics engine
	Demo d=Demo();
	d.start(12, 12, true);

	return 0;
}