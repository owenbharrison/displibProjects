#include <time.h>
#include <algorithm>

#include "Engine.h"
#include "maths/Maths.h"
#include "maths/vector/V3D.h"
using namespace displib;

#include <noise/noise.h>
using namespace noise;

//i really havent a clue where i got this
//i believe it is x and z matrix rotations, without z
V2D projV3D(V3D v, float yaw, float pitch, float zoom) {
	return V2D(
		sinf(yaw)*v.x-cosf(yaw)*v.z,
		(cosf(yaw)*v.x+sinf(yaw)*v.z)*cosf(pitch)+sinf(pitch)*v.y
	)*zoom;
}

class Demo : public Engine {
	public:
	int res, cols, rows;
	float* valField;
	float threshold=0.3f;

	float zInc=0;
	module::Perlin perlinNoise;
	const char* asciiArr=" .,~=#&@";

	int ix(int i, int j) {
		return i+j*cols;
	}

	void setup() override {
		res=2;
		cols=width/res+1;
		rows=height/res+1;
		valField=new float[cols*rows];
		perlinNoise.SetSeed(time(NULL));
	}

	void update(float dt) override {
		threshold=(sinf(totalDeltaTime)+1)/2;
		float xInc=0;
		for (int i=0; i<cols; i++) {
			float yInc=0;
			for (int j=0; j<rows; j++) {
				valField[i+j*cols]=perlinNoise.GetValue(xInc, yInc+100, zInc-100);
				yInc+=0.01f;
			}
			xInc+=0.01f;
		}
		zInc+=dt/4;
	}

	void draw(Raster& rst) override {
		//background
		rst.setChar(' ');
		rst.fillRect(0, 0, width, height);

		//show grid
		rst.setColor(Raster::WHITE);
		for (int i=0; i<cols; i++) {
			for (int j=0; j<rows; j++) {
				float val=valField[i+j*cols];
				int asi=Maths::clamp(val*8, 0, 7);
				rst.setChar(asciiArr[asi]);
				rst.fillRect(i*res, j*res, res, res);
			}
		}


		//marching
		rst.setChar(0x2588);
		rst.setColor(Raster::GREEN);
		for (int i=0; i<cols-1; i++) {
			for (int j=0; j<rows-1; j++) {
				float x=i*res, y=j*res;
				bool tl=valField[ix(i, j)]>threshold;
				bool tr=valField[ix(i+1, j)]>threshold;
				bool bl=valField[ix(i, j+1)]>threshold;
				bool br=valField[ix(i+1, j+1)]>threshold;
				V2D t(x+res/2, y);
				V2D b(x+res/2, y+res);
				V2D l(x, y+res/2);
				V2D r(x+res, y+res/2);
				switch (bl+br*2+tr*4+tl*8) {
					case 1: case 14: rst.drawLine(l, b); break;
					case 2: case 13: rst.drawLine(r, b); break;
					case 3: case 12: rst.drawLine(l, r); break;
					case 4: case 11: rst.drawLine(t, r); break;
					case 5: rst.drawLine(t, l); rst.drawLine(r, b); break;
					case 6: case 9: rst.drawLine(t, b); break;
					case 7: case 8: rst.drawLine(t, l); break;
					case 10: rst.drawLine(t, r); rst.drawLine(l, b); break;
				}
			}
		}
	}
};

int main() {
	//init custom graphics engine
	Demo d=Demo();
	d.startFullscreen(12);

	return 0;
}