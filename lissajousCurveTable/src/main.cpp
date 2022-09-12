#include "Engine.h"
using namespace displib;

class Demo : public Engine {
	public:
	float angle=0;
	int res, cols, rows;

	void setup() override {
		setTitle("Lissajous Curve Table");

		res=45;
		cols=width/res;
		rows=height/res;
	}

	void update(float dt) override {
		angle+=dt/4;
	}

	void draw(Raster& rst) override {
		//reset screen every now and then
		if (updateCount==1||getKey('R')) {

			rst.setChar(' ');
			rst.fillRect(0, 0, width, height);
		}

		rst.setChar(0x2588);
		for (int i=0; i<cols; i++) {
			for (int j=0; j<rows; j++) {
				int cx=i*res+res/2;
				int cy=j*res+res/2;
				float r=res/3;
				float ax=angle*(i+1);
				float ay=angle*(j+1);
				rst.putPixel(cosf(ax)*r+cx, sinf(ay)*r+cy);
			}
		}
	}
};

int main() {
	//init custom graphics engine
	Demo d=Demo();
	d.startWindowed(2, 720, 405);

	return 0;
}